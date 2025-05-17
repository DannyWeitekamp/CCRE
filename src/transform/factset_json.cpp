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

#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1

// #define RAPIDJSON_DEFAULT_ALLOCATOR ::RAPIDJSON_NAMESPACE::MemoryPoolAllocator<::RAPIDJSON_NAMESPACE::CrtAllocator>

// template <typename Encoding=rapidjson::UTF8<>, typename Allocator=RAPIDJSON_DEFAULT_STACK_ALLOCATOR>
// template <typename DocType=rapidjson::Document>

// struct RJSONValuePtr{

// }

namespace cre {

struct FactSetFromJSON_impl{
public:
	inline static const std::string container_prefix = "JSON";
	inline static const bool treat_lists_immutable = true;

	typedef rapidjson::Document::ValueType GV;
	// typedef rapidjson::GenericObject<false, rapidjson::Value> GV;

	typedef rapidjson::Document 	container_t;
	typedef GV::Object			dict_t;
	typedef GV::Array 			list_t;
	typedef GV::Array 			tuple_t;
	typedef GV     				obj_t;
	typedef const GV*     		obj_ptr_t;


	typedef rapidjson::Pointer 	 attr_getter_t;
	typedef GV::MemberIterator   dict_iter_t;
	typedef GV::ValueIterator    list_iter_t;





	// typedef rapidjson::Document 								container_t;
	// typedef rapidjson::GenericObject<false, rapidjson::Value>   dict_t;
	// typedef rapidjson::GenericArray<false, rapidjson::Value> 			list_t;
	// typedef rapidjson::GenericArray<false, rapidjson::Value> 			tuple_t;
	// typedef rapidjson::Value     					obj_t;
	// typedef rapidjson::Pointer 		attr_getter_t;
	// typedef rapidjson::Value::ConstMemberIterator dict_iter_t;
	// typedef rapidjson::Value::ConstValueIterator list_iter_t;

	// typedef rapidjson::GenericMemberIterator<true,Encoding,Allocator>::Iterator   dict_iter_t;
	// typedef const GenericValue*    list_iter_t;
	

	inline static auto& deref_obj_ptr(const auto& x){
		return *x;
	}	

	inline static auto get_obj_ptr(const auto& x){
		// Note: Object is already a pointer wrapper 
		return &x;
	}	

	inline static bool is_dict(const auto& x){return x.IsObject();}
	inline static bool is_list(const auto& x){return x.IsArray();}
	inline static bool is_tuple([[maybe_unused]] 
								 const auto& x){return false;}

	inline static std::string_view to_string_view(const auto& x){
		return std::string_view(x.GetString(), x.GetStringLength());
	}
	inline static auto 			to_dict(const auto& x){
		return x.GetObject();
	}
	inline static auto 			to_list(const auto& x){
		return x.GetArray();
	}
	// inline static tuple_t 			to_tuple(obj_t x){return nb::cast<nb::tuple>(x);}
	inline static Item 				to_item(const auto& x){
		Item item;
		if(x.IsNull()){
			item = Item();
		}else if(x.IsBool()){
			item = Item(x.GetBool());
		}else if(x.IsInt()){
			item = Item(x.GetInt());
		}else if(x.IsUint()){
			item = Item(x.GetUint());
		}else if(x.IsInt64()){
			item = Item(x.GetInt64());
		}else if(x.IsUint64()){
			item = Item(x.GetUint64());
		}else if(x.IsDouble()){
			item = Item(x.GetDouble());
		}else if(x.IsString()){
			std::string_view item_str = std::string_view(x.GetString(), x.GetStringLength());
			item = Item(item_str);
		}
		return item;
	}

	inline static attr_getter_t 	to_attr_getter_t(const std::string_view& x){
		std::string sx = "/" + std::string(x);
		return rapidjson::Pointer(sx.c_str(), sx.size());
	}
	inline static FactType* 		to_fact_type(const auto& x){
		std::string_view type_name = std::string_view(x.GetString(), x.GetStringLength());
		FactType* type = current_context->get_fact_type(type_name);
		return type;
	}

	inline static bool has_attr(const auto& d, const auto& x){
		return x.Get(d) != nullptr;
		// return d.contains(x);
	}

	inline static const obj_t& get_attr(const auto& d, const auto& x){
		return *(x.Get(d));
	}

	

	inline static auto dict_itr_begin(const auto& d){
		return d.MemberBegin();
	}
	inline static auto dict_itr_end(const auto& d){
		return d.MemberEnd();
	}

	inline static auto dict_itr_key_ptr(const auto& itr){
		return &(itr->name);
	}

	inline static auto dict_itr_val_ptr(const auto& itr){
		return &(itr->value);
	}


		// auto& start = d.begin();
		// auto& end = d.end();
		// return std::make_tuple(start, end);
	// }
	inline static auto list_itr_begin(const auto& lst){
		return lst.Begin();
	}
	inline static auto list_itr_end(const auto& lst){
		return lst.End();
	}

	inline static auto list_itr_val_ptr(const auto& itr){
		return &(*itr);
	}

	inline static size_t dict_size(const auto& d){
		return d.MemberCount();
	}

	inline static size_t list_size(const auto& lst){
		return lst.Size();
	}

	inline static std::string obj_to_str(const auto& x){
		try{
			return item_to_string(to_item(x));
		} catch(...) {
			return "Unknown JSON";
		}
		
	}		

};

using FactSetFromJSON = ToFactSetTranslator<FactSetFromJSON_impl>;



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
	// for (auto [key, val] : d) {
		std::string_view fact_id = std::string_view(fact_itr->name.GetString(), fact_itr->name.GetStringLength());
		// std::string_view fact_id = std::string_view(key.GetString(), val.GetStringLength());
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

		ref<Fact> fact_ref = builder.alloc_fact(type, length);
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
				throw std::invalid_argument(error_msg);
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

	// std::string file_content;
	// file_content.reserve(filesize);
	// char* buffer = &file_content[0];
	// size_t readLength = fread(buffer, 1, filesize, fp);
	// buffer[readLength] = '\0';

	char* buffer = new char[filesize + 1];
	size_t readLength = fread(buffer, 1, filesize, fp);
	buffer[readLength] = '\0';
	fclose(fp);

	return buffer;
}

ref<FactSet> FactSet::from_json_file(
	const std::string_view& filename,
	const std::string_view& type_attr,
	const std::string_view& ref_prefix){

	char* buffer = _read_file(filename.data());

	// cout << "buffer: " << buffer << endl;
	rapidjson::Document doc;
	doc.ParseInsitu(buffer);

	ref<FactSet> fact_set = FactSetFromJSON::apply(
		doc, type_attr, ref_prefix
	); 
	// ref<FactSet> fact_set = _FactSet_from_doc(d);
	delete[] buffer;
	return fact_set;
}

ref<FactSet> FactSet::from_json(
	//char* json_str, size_t length,
	const std::string_view& json_str,
	const std::string_view& type_attr,
	const std::string_view& ref_prefix,
	bool copy_buffer){
	// if(length == size_t(-1)){
	// 	length = strlen(json_str);
	// }
	char* buffer;
	if(copy_buffer){
		size_t length = json_str.size();
		buffer = new char[length + 1];
		std::memcpy(buffer, json_str.data(), length);
		buffer[length] = '\0';
	}else{
		buffer = (char*) json_str.data();
	}

	rapidjson::Document doc;
	doc.ParseInsitu(buffer);
	ref<FactSet> fact_set = FactSetFromJSON::apply(
		doc, type_attr, ref_prefix
	); 
	// ref<FactSet> out = _FactSet_from_doc(d);

	if(copy_buffer){
		delete[] buffer;
	}
	return fact_set;
}




//-----------------------------------------------------------------
// : WRTIE JSON from FactSet


struct FactSetJSONWriter {
	bool unique_id_as_key;
	bool doc_as_array;
    std::string_view type_attr;
    std::string_view ref_prefix;
    rapidjson::Document& doc;
    rapidjson::Document::AllocatorType& alloc;

    FactSetJSONWriter(
    	rapidjson::Document& _doc,
    	bool _unique_id_as_key=false,
    	bool _doc_as_array=false,
	    const std::string_view& _type_attr="type",
	    const std::string_view& _ref_prefix="@") :

    	doc(_doc),
    	unique_id_as_key(_unique_id_as_key),
    	doc_as_array(_doc_as_array),
    	type_attr(_type_attr),
    	ref_prefix(_ref_prefix),
    	alloc(_doc.GetAllocator())
    {};

    rapidjson::Value item_to_value(Item item){
		rapidjson::Value item_val;
		switch(item.get_t_id()) {
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
	};

	rapidjson::Value _Fact_to_JSON_obj(Fact* fact){

		rapidjson::Value fact_obj(rapidjson::kObjectType);
		// fact_obj.SetObject();

		// cout << "IS OBJECT0: " << fact_obj.IsObject() << endl;

		FactType* type = fact->type;
		// cout << "IS OBJECT: " << fact_obj.IsObject() << endl;

		size_t i = 0;
		if(type != nullptr){
			// Add the "type" : whatever line
			std::string_view type_name = type->name;
			rapidjson::Value type_attr_val(type_attr.data(), type_attr.size(), alloc);
			rapidjson::Value type_name_val(type_name.data(), type_name.size(), alloc);
			fact_obj.AddMember(type_attr_val, type_name_val, alloc);


			for(; i < type->members.size(); ++i){
				std::string_view attr_name = std::string_view(type->members[i].name);
				rapidjson::Value attr_name_obj(attr_name.data(), attr_name.size(), alloc);

				Item item = fact->get(i);
				rapidjson::Value item_obj = item_to_value(item);

				// cout << " Adding Fact Attr: " << attr_name << " : " << repr_item(item) << endl;
				fact_obj.AddMember(attr_name_obj, item_obj, alloc);
				// cout << "AFTE Fact Attr: " << attr_name << endl;
			}
		}
		// cout << "i: " << i << " " << (uint64_t) type << endl;
		for(; i < fact->length; ++i){
			std::string attr_name = std::to_string(i);
			rapidjson::Value attr_name_obj(attr_name.c_str(), attr_name.size(), alloc);

			Item item = fact->get(i);
			rapidjson::Value item_obj = item_to_value(item);

			// cout << " Adding Fact Attr: " << attr_name << " : " << repr_item(item) << endl;
			fact_obj.AddMember(attr_name_obj, item_obj, alloc);
			// cout << "AFTE Fact Attr: " << attr_name << endl;
		}
		return fact_obj;
	};

	rapidjson::Value _Fact_to_JSON_array(Fact* fact){
		rapidjson::Value fact_array(rapidjson::kArrayType);
			
		for(size_t i=0; i < fact->length; ++i){
			Item item = fact->get(i);
			rapidjson::Value item_obj = item_to_value(item);

			fact_array.PushBack(item_obj, alloc);
		}
		return fact_array;
	};

	void  _doc_add_fact_member(Fact* fact){
		// Make the key
		std::string fact_id;
		if(unique_id_as_key) {
			fact_id=fact->get_unique_id();
		}
		if(fact_id.size() == 0){
			fact_id = std::to_string(fact->f_id);
		}
		rapidjson::Value fact_key_val(fact_id.data(), fact_id.size(), alloc);
		
		// Make the value (object or array)
		rapidjson::Value fact_val;
		if(fact->type != nullptr || !fact->immutable){
			fact_val = _Fact_to_JSON_obj(fact);
		}else{
			// Untyped immutable facts written as arrays
			fact_val = _Fact_to_JSON_array(fact);
		}

		// Add the member
		doc.AddMember(fact_key_val, fact_val, alloc);
	};

	void  _doc_push_back(Fact* fact){
		// Make the key
		std::string fact_id;
		
		// Make the value (object or array)
		rapidjson::Value fact_val;
		if(fact->type != nullptr || !fact->immutable){
			fact_val = _Fact_to_JSON_obj(fact);
		}else{
			// Untyped immutable facts written as arrays
			fact_val = _Fact_to_JSON_array(fact);
		}

		// Add the member
		doc.PushBack(fact_val, alloc);	
	};

	std::string apply(FactSet* fs){
		

		// alloc = _doc.GetAllocator();
		// rapidjson::Document::AllocatorType& alloc = d.GetAllocator();

		if(!doc_as_array){
			doc.SetObject();
			for (auto it = fs->begin(); it != fs->end(); ++it) {
				_doc_add_fact_member(*it);
			}	
		}else{
			doc.SetArray();
			for (auto it = fs->begin(); it != fs->end(); ++it) {
				_doc_push_back(*it);
			}
		}	
		
		rapidjson::StringBuffer buffer;
	    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	    doc.Accept(writer);

	    std::string json_str = std::string(buffer.GetString(), buffer.GetSize());
		return json_str;
	};
};

std::string FactSet::to_json(
		bool unique_id_as_key,
		bool doc_as_array,
        const std::string_view& type_attr,
        const std::string_view& ref_prefix
    ){

	rapidjson::Document doc;
	auto writer = FactSetJSONWriter(
		doc,
		unique_id_as_key, doc_as_array,
		type_attr, ref_prefix
	);
	std::string json_str = writer.apply(this);
	// cout << "Before Write Doc" << endl;
	// rapidjson::Document d = _FactSet_to_doc(this);
	// cout << "After Write Doc" << endl;

	// rapidjson::StringBuffer buffer;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    // d.Accept(writer);
	
    // // Create a copy of the string that can be freed by the caller
    // std::string json_str = std::string(buffer.GetString(), buffer.GetSize());

    // size_t length = buffer.GetSize();
    // char* json_cstr = new char[length + 1];
    // std::memcpy(json_cstr, buffer.GetString(), length);
    // json_cstr[length] = '\0';

    
	
    return json_str;
}

} // NAMESPACE_END(cre)



