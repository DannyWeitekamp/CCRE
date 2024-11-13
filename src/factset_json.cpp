#include "../include/types.h"
#include "../include/context.h"
#include "../external/rapidjson/reader.h"
#include "../external/rapidjson/filereadstream.h"
#include "../external/rapidjson/document.h"
#include "../external/rapidjson/pointer.h"
#include "../external/rapidjson/stringbuffer.h"
#include "../external/rapidjson/writer.h"
#include "../include/factset.h"
#include "../include/item.h"
#include "../include/intern.h"
#include "../include/ref.h"


#include <iostream>
#include <algorithm>

//-----------------------------------------------------------------
// : READ JSON to FactSet

// struct FactSetFromJSON_impl{
// 	// using container_t =  nb::handle;
// 	// using dict_t = 	     nb::dict;
// 	// using list_t = 	     nb::list;
// 	// using tuple_t = 	 nb::tuple;
// 	// using obj_t = 	     nb::handle;
// 	// using attr_getter_t =nb::str;

// 	typedef const nb::handle container_t;
// 	typedef const nb::dict dict_t;
// 	typedef const nb::list list_t;
// 	typedef const nb::tuple tuple_t;
// 	typedef const nb::handle obj_t;
// 	typedef const nb::str attr_getter_t;

// 	inline static bool is_dict(obj_t x){return nb::isinstance<nb::dict>(x);}
// 	inline static bool is_list(obj_t x){return nb::isinstance<nb::list>(x);}
// 	inline static bool is_tuple(obj_t x){return nb::isinstance<nb::tuple>(x);}

// 	inline static std::string_view to_string_view(obj_t x){return nb::cast<std::string_view>(x);}
// 	inline static dict_t 			to_dict(obj_t x){return nb::cast<nb::dict>(x);}
// 	inline static list_t 			to_list(obj_t x){return nb::cast<nb::list>(x);}
// 	inline static tuple_t 			to_tuple(obj_t x){return nb::cast<nb::tuple>(x);}
// 	inline static Item 			to_item(obj_t x){return Item_from_py(x);}
// 	inline static attr_getter_t 	to_attr_getter_t(const std::string_view& x){return nb::str(x.data(), x.size());}
// 	inline static FactType* 		to_fact_type(obj_t x){return FactType_from_py(x);}

// 	inline static bool has_attr(dict_t d, attr_getter_t x){
// 		return d.contains(x);
// 	}

// 	inline static obj_t get_attr(dict_t d, attr_getter_t x){
// 		return d[x];
// 	}
// };

// using FactSetFromJSON = ToFactSetTranslator<FactSetFromJSON_impl>;



ref<FactSet>  _FactSet_from_doc(rapidjson::Document& d){
	if(!d.IsObject()){
		throw std::runtime_error("Input JSON is not an object.");
	}
	// Globals
	auto type_ptr = rapidjson::Pointer("/type");
	CRE_Context* context = current_context;

	// --------------
	// : First Pass: Mapping + Finding Size

	std::vector<std::tuple<rapidjson::GenericObject<false, rapidjson::Value>, FactType*, size_t, size_t>> fact_infos;
	HashMap<std::string_view, size_t> fact_map = {};

	fact_infos.reserve(d.MemberCount());
	fact_map.reserve(d.MemberCount());
	size_t byte_offset = 0;
	size_t index = 0;
	for (auto fact_itr = d.MemberBegin(); fact_itr != d.MemberEnd(); ++fact_itr){
		std::string_view fact_id = std::string_view(fact_itr->name.GetString(), fact_itr->name.GetStringLength());
		// cout << fact_id << endl;
		if(!fact_itr->value.IsObject()){
			throw std::runtime_error("Fact JSON with key " + std::string(fact_id) + " is not an object.");
		}

		auto fact_obj = fact_itr->value.GetObject();

		size_t length = fact_obj.MemberCount();
		FactType* type = nullptr;
		auto type_obj = type_ptr.Get(fact_obj);
		if(type_obj != nullptr){
			std::string_view type_name = std::string_view(type_obj->GetString(), type_obj->GetStringLength());
			type = context->get_fact_type(type_name);

			// if(type == nullptr){
			// 	throw std::runtime_error("Fact type '" + std::string(type_name) + "' not defined in CRE_Context: " + context->name);
			// };
			if(type->members.size() > length){
				length = type->members.size();
			}
			length -= 1; // Don't count type in the count
		}
		if(type != nullptr){
			length = std::max(length, size_t(type->members.size()));
		}
		fact_infos.push_back({fact_obj,type,length,byte_offset});
		auto [it, inserted] = fact_map.insert({fact_id, index});
		// cout << fact_id << endl;
		if(!inserted){
			throw std::runtime_error("Duplicate fact identifier: " + std::string(fact_id));
		}
		byte_offset += SIZEOF_FACT(length);//sizeof(Fact) + sizeof(Item) * length;
		index++;
	}

	FactSetBuilder builder = FactSetBuilder(d.MemberCount(), byte_offset);

	// --------------
	// : Second Pass: Building Each object

	for(auto& fact_info : fact_infos){
		auto& fact_obj = std::get<0>(fact_info);
		FactType* type = std::get<1>(fact_info);
		size_t length = std::get<2>(fact_info);
		// size_t offset = std::get<3>(fact_info);

		ref<Fact> fact_ref = builder.add_empty(length, type);
		Fact* __restrict fact = fact_ref.get();
		fact->type = type;
		// cout << uint64_t(builder->alloc_buffer->data) + offset << endl;
		// cout << uint64_t(fact) << endl;
		// cout << endl;
		



		for (auto member_itr = fact_obj.MemberBegin(); member_itr != fact_obj.MemberEnd(); ++member_itr){
			std::string_view key = std::string_view(member_itr->name.GetString(), member_itr->name.GetStringLength());

			// Ignore the 'type' member (handled above)
			if(key == "type"){
				continue;
			}

			// If the key is a number then used it as the index
			char* p;
			int64_t index = strtol (key.data(),&p,10);
			if(*p != 0) index = -1;

			// Otherwise get the key's index from the fact type
			if(type != nullptr){
				index = type->get_attr_index(key);
			}

			// Throw error if member index cannot be resolved
			if(index == -1){
				std::string type_str = type != nullptr ? std::string(type->name) : "NULL";
				std::string error_msg = "Key '" + std::string(key) + "' is not an integer or named member of fact type '" + type_str + "'.";
				throw std::runtime_error(error_msg);
			}

			// Populate the item
			Item item;
			if(member_itr->value.IsNull()){
				item = Item();
			}else if(member_itr->value.IsBool()){
				item = Item(member_itr->value.GetBool());
			}else if(member_itr->value.IsInt()){
				item = Item(member_itr->value.GetInt());
			}else if(member_itr->value.IsUint()){
				item = Item(member_itr->value.GetUint());
			}else if(member_itr->value.IsInt64()){
				item = Item(member_itr->value.GetInt64());
			}else if(member_itr->value.IsUint64()){
				item = Item(member_itr->value.GetUint64());
			}else if(member_itr->value.IsDouble()){
				item = Item(member_itr->value.GetDouble());
			}else if(member_itr->value.IsString()){
				std::string_view item_str = std::string_view(member_itr->value.GetString(), member_itr->value.GetStringLength());

				// Reference to another fact
				if(item_str.length() > 0 && item_str[0] == '@'){
					std::string_view ref_str = std::string_view(item_str.data()+1, item_str.length()-1);
					size_t index = fact_map[ref_str];
					size_t offset = std::get<3>(fact_infos[index]);
					Fact* fact_ptr = (Fact*)(builder.alloc_buffer->data + offset);
					item = Item(fact_ptr);

				// Normal String Case
				}else{
					item = Item(item_str);					
				}
			}
			fact->set(index, item);
		}

		// _init_fact(fact, length, type);
		// fact->alloc_buffer = builder.alloc_buffer;
		builder.fact_set->_declare_back(fact_ref);
		// _declare_to_empty(builder.fact_set, fact, length, type);
	}
	// builder.alloc_buffer->add_ref(fact_infos.size());
	ref<FactSet> fact_set = builder.fact_set;
	// delete builder;
	return fact_set;
}

char* _read_file(const char* filename){
	FILE* fp = fopen(filename, "rb"); // non-Windows use "r"
	
	fseek(fp, 0, SEEK_END);
	size_t filesize = (size_t)ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = (char*)malloc(filesize + 1);
	size_t readLength = fread(buffer, 1, filesize, fp);
	buffer[readLength] = '\0';
	fclose(fp);

	return buffer;
}

ref<FactSet> FactSet::from_json_file(const char* filename){
	char* buffer = _read_file(filename);

	// cout << "buffer: " << buffer << endl;
	rapidjson::Document d;
	d.ParseInsitu(buffer);
	ref<FactSet> fact_set = _FactSet_from_doc(d);
	// FactSet* fact_set = new FactSet();
	delete[] buffer;
	return fact_set;
}

ref<FactSet> FactSet::from_json(char* json_str, size_t length, bool copy_buffer){
	if(length == size_t(-1)){
		length = strlen(json_str);
	}
	char* buffer;
	if(copy_buffer){
		buffer = new char[length + 1];
		std::memcpy(buffer, json_str, length);
		buffer[length] = '\0';
	}else{
		buffer = json_str;
	}

	rapidjson::Document d;
	d.ParseInsitu(buffer);
	ref<FactSet> out = _FactSet_from_doc(d);

	if(copy_buffer){
		delete[] buffer;
	}
	return out;
}


//-----------------------------------------------------------------
// : WRTIE JSON from FactSet



rapidjson::Value item_to_value(Item item, rapidjson::Document::AllocatorType& alloc){
	rapidjson::Value item_val;
	switch(item.t_id) {
        case T_ID_BOOL:
            item_val = rapidjson::Value(item.as_bool());
            break;
        case T_ID_INT:
			item_val = rapidjson::Value(item.as_int());
            break;
        case T_ID_FLOAT:
			item_val = rapidjson::Value(item.as_float());
            break;
        case T_ID_STR:
			{
				std::string_view item_str = item.as_string();
				item_val = rapidjson::Value(item_str.data(), item_str.size(), alloc);
			}
            break;
        case T_ID_FACT:
			{
				Fact* fact = (Fact*) item.val;
				std::string ref_str = "@" + std::to_string(fact->f_id);
            	item_val = rapidjson::Value(ref_str.c_str(), ref_str.size(), alloc);
			}
			break;
			
		default:
			throw std::runtime_error("Invalid item type.");
    }
	return item_val;
}

rapidjson::Document  _FactSet_to_doc(FactSet* fs){
	rapidjson::Document d;
	rapidjson::Document::AllocatorType& alloc = d.GetAllocator();
	d.SetObject();

	// vector<std::string> all_allocated_strings = {};
	for (auto it = fs->begin(); it != fs->end(); ++it) {
		Fact* fact = *it;
		// cout << "f_id" << fact->f_id;
		std::string fact_id = std::to_string(fact->f_id);
		rapidjson::Value fact_id_obj(fact_id.c_str(), fact_id.size(), alloc);
		rapidjson::Value fact_obj(rapidjson::kObjectType);
		// fact_obj.SetObject();

		// cout << "IS OBJECT0: " << fact_obj.IsObject() << endl;

		FactType* type = fact->type;
		// cout << "IS OBJECT: " << fact_obj.IsObject() << endl;

		size_t i = 0;
		if(type != nullptr){
			// Add the "type" : whatever line
			std::string_view type_name = type->name;
			rapidjson::Value _type_obj("type", alloc);
			rapidjson::Value type_name_obj(type_name.data(), type_name.size(), alloc);
			fact_obj.AddMember(_type_obj, type_name_obj, alloc);


			for(; i < type->members.size(); ++i){
				std::string_view attr_name = std::string_view(type->members[i].name);
				rapidjson::Value attr_name_obj(attr_name.data(), attr_name.size(), alloc);

				Item item = *fact->get(i);
				rapidjson::Value item_obj = item_to_value(item, alloc);

				// cout << " Adding Fact Attr: " << attr_name << " : " << repr_item(item) << endl;
				fact_obj.AddMember(attr_name_obj, item_obj, alloc);
				// cout << "AFTE Fact Attr: " << attr_name << endl;
			}
		}
		// cout << "i: " << i << " " << (uint64_t) type << endl;
		for(; i < fact->length; ++i){
			std::string attr_name = std::to_string(i);
			rapidjson::Value attr_name_obj(attr_name.c_str(), attr_name.size(), alloc);

			Item item = *fact->get(i);
			rapidjson::Value item_obj = item_to_value(item, alloc);

			// cout << " Adding Fact Attr: " << attr_name << " : " << repr_item(item) << endl;
			fact_obj.AddMember(attr_name_obj, item_obj, alloc);
			// cout << "AFTE Fact Attr: " << attr_name << endl;
		}
			


			// Item* item = fact->get(i);
			// std::string item_str = item_to_string(item);
			// rapidjson::Value item_obj(item_str.c_str(), item_str.size(), d.GetAllocator());
			// fact_obj.AddMember(rapidjson::GenericStringRef<char>(std::to_string(i).c_str()), item_obj, d.GetAllocator());
		

		// cout << "Adding member: " << fact_id << endl;
		d.AddMember(fact_id_obj, fact_obj, alloc);
		// cout << "AFTER Adding member: " << fact_id << endl;
	}
	return d;
};

char* FactSet::to_json(FactSet* fs){

	// cout << "Before Write Doc" << endl;
	rapidjson::Document d = _FactSet_to_doc(fs);
	// cout << "After Write Doc" << endl;

	rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
	
	// cout << "BLARG: " << length << endl;
    // Create a copy of the string that can be freed by the caller
    size_t length = buffer.GetSize();
    char* json_str = new char[length + 1];
    std::memcpy(json_str, buffer.GetString(), length);
    json_str[length] = '\0';
	
    return json_str;
}




