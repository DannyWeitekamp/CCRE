#ifndef _CRE_UNICODE_H_
#define _CRE_UNICODE_H_

#include <string>
#include <iostream>
#include <sstream> 
#include <bit>
#include <cstdint>
#include <inttypes.h>
#include <vector>
#include <tuple>
#include <list>
#include <unordered_map>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <iostream>
using namespace std;


// struct intern_bucket{
// 	// uint32_t s_id;
// 	uint32_t refcount; 
// };

// struct MemInfo {
//     std::atomic_size_t     refct;
//     NRT_dtor_function dtor;
//     void              *dtor_info;
//     void              *data;
//     size_t            size;    /* only used for NRT allocated memory */
//     NRT_ExternalAllocator *external_allocator;
// };
// }
	
struct InternStr{
	uint32_t length;
	uint32_t refcount;
	char* data;
	InternStr(uint32_t _length, uint32_t _refcount, char* _data){
		length = _length;
		refcount = _refcount;
		data = _data;
	}
};

struct nb_unicode{
	char* data;
	int64_t length;
	int32_t kind;
	uint32_t is_ascii;
	uint64_t hash;
	void* meminfo;
	void* parent;
};

unordered_map<string, InternStr*> intern_map = {};
// vector<string> s_id_map = {};
// list<uint32_t> free_s_ids = {};
// intern_bucket null_bucket = {0,0};

string s_id_to_string(uint64_t s_id){
	auto key_val = bit_cast<pair<string,InternStr*>*>(s_id);
	return key_val->first;
}

uint32_t intern_refcount(uint64_t s_id){
	auto key_val = bit_cast<pair<string,InternStr*>*>(s_id);
	return key_val->second->refcount;
}

uint32_t intern_refcount(string& s){
	auto itr = intern_map.find(s);
	if(itr == intern_map.end()){
		return 0;
	}else{
		return itr->second->refcount; 
	}
}

tuple<InternStr*,char*> intern(const string& s){

	// cout << "INTERN START" << endl;
	auto itr = intern_map.find(s);

	string key_str;
	uint32_t refcount;
	InternStr* intern_str;
	if(itr == intern_map.end()){
		auto L = s.length();
		char* data = (char*) malloc(sizeof(InternStr) + (sizeof(char) * L));
		char* char_data = (char*)(data + sizeof(InternStr));

		// cout << "SIZE:" << sizeof(InternStr) + (sizeof(char) * L) << endl; 
		// string key_str;
		key_str.assign(char_data, L);

		InternStr* intern_str = (InternStr*) data;
		intern_str->length = L; 
		intern_str->refcount = 1; 
		intern_str->data = char_data; 
		 //new InternStr(s.length(), 1, s.data());
		// refcount = 1;
		itr = intern_map.emplace(make_pair(key_str, intern_str)).first;
		// itr = intern_map.insert(make_pair(key_str, intern_str)).first;

		// interned_str = itr->first;

	// Otherwise s_id is ;
	}else{
		intern_str = itr->second;
		intern_str->refcount += 1;
	}
	// cout << "INTERN END" << endl;
	return make_tuple(intern_str, intern_str->data);
}

// void decref_intern(string& s){
// 	auto itr = intern_map.find(s);
// 	if(itr != intern_map.end()){
// 		itr->second -= 1;
// 		uint32_t refcount = itr->second;
// 		if(refcount == 0){
// 			intern_map.erase(itr);
// 		}
// 	}
// }

// void incref_intern(string& s){
// 	auto itr = intern_map.find(s);
// 	if(itr != intern_map.end()){
// 		itr->second += 1;
// 	}
// }

// void decref_intern(uint64_t s_id){
// 	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
// 	// auto itr = bit_cast<unordered_map<string, uint32_t>::const_iterator>(key_val);
// 	// auto b = intern_map.begin();
// 	// auto itr = b + std::distance(bit_cast<pair<string,uint32_t>*>(&(*b)), key_val);

// 	key_val->second -= 1;
// 	uint32_t refcount = key_val->second;
// 	if(refcount == 0){
// 		// NOTE: This step is slow there may be a way around it
// 		//	since we already have a pointer to the key-value pair
// 		auto str = key_val->first;
// 		intern_map.erase(str);
// 	}
// }

// void incref_intern(uint64_t s_id){
// 	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
// 	key_val->second += 1;	
// }




#endif /* _CRE_UNICODE_H_ */
