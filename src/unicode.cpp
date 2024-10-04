
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
#include <cstring>
#include <iostream>
#include "../include/unicode.h"
#include "../include/hash.h"
#include "../include/unordered_dense.h"

using namespace std;

// unordered_flat_map<string, uint32_t> intern_map = {};
// unordered_map<string, uint32_t> intern_map = {};
// vector<string> s_id_map = {};
// list<uint32_t> free_s_ids = {};
// intern_bucket null_bucket = {0,0};
ankerl::unordered_dense::map<string, uint32_t, CREHash> intern_map = {};


string s_id_to_string(uint64_t s_id){
	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
	return key_val->first;
}

uint32_t intern_refcount(uint64_t s_id){
	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
	return key_val->second;
}

uint32_t intern_refcount(string& s){
	auto itr = intern_map.find(s);
	if(itr == intern_map.end()){
		return 0;
	}else{
		return itr->second; 
	}
}

tuple<uint64_t, const char*> intern(string& s){
	auto itr = intern_map.find(s);

	
	uint32_t refcount;
	if(itr == intern_map.end()){
		refcount = 1;
		itr = intern_map.insert({s, refcount}).first;
		// interned_str = itr->first;

	// Otherwise s_id is ;
	}else{
		// interned_str = itr->first;
		itr->second += 1;
	}
	// string_view interned_str = string_view(itr->first->data(),itr->first->length()); 

	// cout << "INTERN" << interned_str;
	const char* data_ptr = itr->first.data();
	return make_tuple(bit_cast<uint64_t>(&(*itr)), data_ptr);
}

void decref_intern(string& s){
	auto itr = intern_map.find(s);
	if(itr != intern_map.end()){
		itr->second -= 1;
		uint32_t refcount = itr->second;
		if(refcount == 0){
			intern_map.erase(itr);
		}
	}
}

void incref_intern(string& s){
	auto itr = intern_map.find(s);
	if(itr != intern_map.end()){
		itr->second += 1;
	}
}

void decref_intern(uint64_t s_id){
	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
	// auto itr = bit_cast<unordered_map<string, uint32_t>::const_iterator>(key_val);
	// auto b = intern_map.begin();
	// auto itr = b + std::distance(bit_cast<pair<string,uint32_t>*>(&(*b)), key_val);

	key_val->second -= 1;
	uint32_t refcount = key_val->second;
	if(refcount == 0){
		// NOTE: This step is slow there may be a way around it
		//	since we already have a pointer to the key-value pair
		auto str = key_val->first;
		intern_map.erase(str);
	}
}

void incref_intern(uint64_t s_id){
	auto key_val = bit_cast<pair<string,uint32_t>*>(s_id);
	key_val->second += 1;	
}
