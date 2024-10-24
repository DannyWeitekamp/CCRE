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


#include <iostream>
#include <algorithm>

//-----------------------------------------------------------------
// : READ JSON to FactSet

FactSet*  _FactSet_from_doc(rapidjson::Document& d){
	if(!d.IsObject()){
		throw std::runtime_error("Input JSON is not an object.");
	}

	auto type_ptr = rapidjson::Pointer("/type");

	CRE_Context* context = default_context;

	std::vector<std::tuple<rapidjson::GenericObject<false, rapidjson::Value>, FactType*, size_t, size_t>> fact_infos;
	ankerl::unordered_dense::map<std::string_view, size_t, CREHash, std::equal_to<>> fact_map = {};

	fact_infos.reserve(d.MemberCount());
	fact_map.reserve(d.MemberCount());
	size_t byte_offset = 0;
	size_t index = 0;
	for (auto fact_itr = d.MemberBegin(); fact_itr != d.MemberEnd(); ++fact_itr){
		std::string_view fact_id = std::string_view(fact_itr->name.GetString(), fact_itr->name.GetStringLength());
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

			if(type == nullptr){
				throw std::runtime_error("Fact type '" + std::string(type_name) + "' not defined in CRE_Context: " + context->name);
			};
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
		if(!inserted){
			throw std::runtime_error("Duplicate fact identifier: " + std::string(fact_id));
		}
		byte_offset += sizeof(Fact) + sizeof(Item) * length;
		index++;
	}

	FactSetBuilder builder = FactSetBuilder(d.MemberCount(), byte_offset);


	for(auto& fact_info : fact_infos){
		auto& fact_obj = std::get<0>(fact_info);
		FactType* type = std::get<1>(fact_info);
		size_t length = std::get<2>(fact_info);
		// size_t offset = std::get<3>(fact_info);

		Fact* fact = builder.next_empty(length);
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
				item = empty_item();
			}else if(member_itr->value.IsBool()){
				item = to_item(member_itr->value.GetBool());
			}else if(member_itr->value.IsInt()){
				item = to_item(member_itr->value.GetInt());
			}else if(member_itr->value.IsUint()){
				item = to_item(member_itr->value.GetUint());
			}else if(member_itr->value.IsInt64()){
				item = to_item(member_itr->value.GetInt64());
			}else if(member_itr->value.IsUint64()){
				item = to_item(member_itr->value.GetUint64());
			}else if(member_itr->value.IsDouble()){
				item = to_item(member_itr->value.GetDouble());
			}else if(member_itr->value.IsString()){
				std::string_view item_str = std::string_view(member_itr->value.GetString(), member_itr->value.GetStringLength());

				// Reference to another fact
				if(item_str.length() > 0 && item_str[0] == '@'){
					std::string_view ref_str = std::string_view(item_str.data()+1, item_str.length()-1);
					size_t index = fact_map[ref_str];
					size_t offset = std::get<3>(fact_infos[index]);
					Fact* fact_ptr = (Fact*)(builder.alloc_buffer.data + offset);
					item = fact_to_item(fact_ptr);

				// Normal String Case
				}else{
					item = str_to_item(item_str);					
				}
			}
			fact->set(index, item);
		}
		_declare_from_empty(builder, fact, length, type);
	}
	FactSet* fact_set = builder.fact_set;
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

extern "C" FactSet* FactSet_from_json_file(const char* filename){
	char* buffer = _read_file(filename);

	// cout << "buffer: " << buffer << endl;
	rapidjson::Document d;
	d.ParseInsitu(buffer);
	FactSet* fact_set = _FactSet_from_doc(d);
	// FactSet* fact_set = new FactSet();
	delete[] buffer;
	return fact_set;
}

extern "C" FactSet* FactSet_from_json(char* json_str, size_t length, bool copy_buffer){
	if(length == size_t(-1)){
		length = strlen(json_str);
	}
	char* buffer;
	if(copy_buffer){
		buffer = (char*)malloc(length + 1);
		std::memcpy(buffer, json_str, length);
		buffer[length] = '\0';
	}else{
		buffer = json_str;
	}

	rapidjson::Document d;
	d.ParseInsitu(buffer);
	return _FactSet_from_doc(d);
}


//-----------------------------------------------------------------
// : WRTIE JSON from FactSet



rapidjson::Value item_to_value(Item item, rapidjson::Document::AllocatorType& alloc){
	rapidjson::Value item_val;
	switch(item.t_id) {
        case T_ID_BOOL:
            item_val = rapidjson::Value(item_get_bool(item));
            break;
        case T_ID_INT:
			item_val = rapidjson::Value(item_get_int(item));
            break;
        case T_ID_FLOAT:
			item_val = rapidjson::Value(item_get_float(item));
            break;
        case T_ID_STR:
			{
				std::string_view item_str = item_get_string(item);
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
		std::string fact_id = std::to_string(fact->f_id);
		rapidjson::Value fact_id_obj(fact_id.c_str(), fact_id.size(), alloc);
		rapidjson::Value fact_obj(rapidjson::kObjectType);
		// fact_obj.SetObject();

		// cout << "IS OBJECT0: " << fact_obj.IsObject() << endl;

		
		
		FactType* type = fact->type;

		// cout << "IS OBJECT: " << fact_obj.IsObject() << endl;

		size_t i = 0;
		if(type != nullptr){
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

extern "C" char* FactSet_to_json(FactSet* fs){

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




