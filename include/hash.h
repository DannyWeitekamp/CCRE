#ifndef _CRE_HASH_H_
#define _CRE_HASH_H_

#include <cstdint>
#include <string>
// #include "../include/item.h"
// #include "../include/fact.h"
#include "../include/unordered_dense.h"
// #include "../include/types.h"

struct Item;
class Fact;

// using namespace std;

extern const uint64_t _PyHASH_XXPRIME_1;
extern const uint64_t _PyHASH_XXPRIME_2;
extern const uint64_t _PyHASH_XXPRIME_5;
extern const uint64_t siphash_k0_default;
extern const uint64_t siphash_k1_default;

uint64_t _PyHASH_XXROTATE(uint64_t x);



uint64_t _Py_HashDouble(double v);
uint64_t hash_bytes(const uint8_t *val, uint64_t _len);
uint64_t fnv1a(const uint8_t *val, uint64_t _len);
uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed );
uint64_t hash_string(const std::string &val);

// Python's siphash24 
uint64_t siphash24(uint64_t k0, uint64_t k1, const void* src, uint64_t src_sz);


uint64_t accum_item_hash(uint64_t acc, uint64_t lane);
uint64_t process_return(uint64_t val);

struct CREHash {
    using is_transparent = void;
    using key_equal = std::equal_to<>;  // Pred to use

    uint64_t operator()(const bool& x) const {
        return x;
    }

    uint64_t operator()(const uint64_t & x) const {
        return x;
    }

    uint64_t operator()(const int64_t & x) const {
        return x;
    }

    uint64_t operator()(const double& x) const {
        return _Py_HashDouble(x);
    }

    uint64_t operator()(const std::string_view& x) const {
        // return fnv1a((const uint8_t*) x.data(), x.size());
        return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
    }

    uint64_t operator()(const std::string& x) const {
        // return fnv1a((const uint8_t*) x.c_str(), x.length());
        return MurmurHash64A((const uint8_t*) x.c_str(), x.length(), 0xFF);
    }

    uint64_t operator()(Item& x); 

    // Not Const
    uint64_t operator()(Fact* x); 
};


template<class T, class U>
using HashMap = ankerl::unordered_dense::map<T, U, CREHash, std::equal_to<>>;

template<class T>
using HashSet = ankerl::unordered_dense::set<T, CREHash, std::equal_to<>>;



#endif /* _CRE_HASH_H_ */
