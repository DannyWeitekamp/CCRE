#ifndef _CRE_INTERN_H_
#define _CRE_INTERN_H_

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
#include "../include/hash.h"
#include "../include/unordered_dense.h"
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
	// InternStr(uint32_t _length, uint32_t _refcount, char* _data){
	// 	length = _length;
	// 	refcount = _refcount;
	// 	data = _data;
	// }
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

// struct string_hash
// {
//     using hash_type = std::hash<std::string_view>;
//     using is_transparent = void;
 
//     std::size_t operator()(const char* str) const        { return hash_type{}(str); }
//     std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
//     std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
// };



// using namespace std;

// unordered_flat_map<string, uint32_t> intern_map = {};
// unordered_map<string, uint32_t> intern_map = {};
// vector<string> s_id_map = {};
// list<uint32_t> free_s_ids = {};
// intern_bucket null_bucket = {0,0};
// ankerl::unordered_dense::map<std::string_view, InternStr*, CREHash> intern_map = {};
// ankerl::unordered_dense::map<std::string, InternStr*, CREHash, std::equal_to<>> intern_map;

// unordered_map<std::string, InternStr*> intern_map = {};

ostream& operator<<(std::ostream& out, InternStr fs);
std::string_view intern(const std::string_view& sv);
std::pair<std::string_view, uint64_t> intern_ret_hash(const std::string_view& sv);
extern ankerl::unordered_dense::set<std::string_view, CREHash, std::equal_to<>> intern_set;
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




#endif /* _CRE_INTERN_H_ */
