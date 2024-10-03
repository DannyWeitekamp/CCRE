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
#include <cstring>
#include <iostream>
#include "../include/unicode.h"

using namespace std;

// #include <boost/unordered/unordered_flat_map.hpp>

// using namespace boost::unordered;


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
	char* data;
	uint32_t length;
	uint32_t s_id;
	uint64_t hash;
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

tuple<uint64_t, const char*> intern(string& s);





#endif /* _CRE_UNICODE_H_ */
