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


struct intern_bucket{
	uint32_t s_id;
	uint32_t refcount; 
};

unordered_map<string, uint32_t> intern_map = {};
vector<tuple<string,uint32_t>> s_id_map = {};
list<uint32_t> free_s_ids = {};


intern_bucket null_bucket = {0,0};

uint32_t intern_refcount(string s){
	auto itr = intern_map.find(s);

	// If failed to find string, make a new bucket;
	if(itr == intern_map.end()){
		return 0;
	// Otherwise s_id is ;
	}else{
		uint32_t s_id = itr->second;
		uint32_t refcount = get<1>(s_id_map[s_id]);
		// intern_bucket bucket = s_id_map[s_id];;
		return refcount; 
	}
}

tuple<uint32_t,char*> intern(string s){
	auto itr = intern_map.find(s);

	// If failed to find string, make a new bucket;
	uint32_t s_id;
	// intern_bucket bucket;
	string interned_str;
	if(itr == intern_map.end()){
		bool no_free = free_s_ids.empty();
		if(no_free){
			s_id = intern_map.size();
		}else{
			s_id = free_s_ids.front();
			free_s_ids.pop_front();
		}
		// bucket = {s_id, 1};

		auto ins_itr = intern_map.insert({s, s_id}).first;
		interned_str = ins_itr->first;

		if(no_free){
			s_id_map.push_back(make_tuple(interned_str,1));
		}else{
			s_id_map[s_id] = make_tuple(interned_str,1);
		}


	// Otherwise s_id is ;
	}else{
		interned_str = itr->first;
		s_id = itr->second;
		auto tup = s_id_map[s_id];
		s_id_map[s_id] = {get<0>(tup), get<1>(tup)+1};
		itr->second = s_id;//#, bucket.refcount+1};
		// s_id = bucket.s_id;
	}
	return make_tuple(s_id, interned_str.data());
}

string s_id_to_string(uint32_t s_id){
	return get<0>(s_id_map[s_id]);
}

void free_s_id(string s_id){


}



#endif /* _CRE_UNICODE_H_ */
