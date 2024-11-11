#ifndef _CRE_HASH_H_
#define _CRE_HASH_H_

#include <cstdint>
#include <string>
// #include <bit>
#include <unordered_map>
#include "../include/unordered_dense.h"


// NOTE: Do not import types.h here

// Forward Declarations
struct Item;
class Fact;
class FactView;
class UintPair;
uint64_t hash_item(const Item& x);


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

    template <std::integral T>
    uint64_t operator()(const T& x) const {
        return x;
    }

    template <std::floating_point T>
    uint64_t operator()(const T& x) const {
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

    uint64_t operator()(const Item& x) const{
        return hash_item(x);
    }

    uint64_t operator()(const FactView& x) const;

    // Not Const
    uint64_t operator()(Fact* x);

    uint64_t operator()(const UintPair& x) const;
    

};


template<class T, class U>
// using HashMap = std::unordered_map<T, U, CREHash, std::equal_to<>>;
using HashMap = ankerl::unordered_dense::map<T, U, CREHash, std::equal_to<>>;

template<class T>
using HashSet = ankerl::unordered_dense::set<T, CREHash, std::equal_to<>>;



#endif /* _CRE_HASH_H_ */
