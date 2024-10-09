#ifndef _CRE_HASH_H_
#define _CRE_HASH_H_

#include <cstdint>
#include <string>
#include "../include/item.h"
#include "../include/types.h"

using namespace std;

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
uint64_t hash_string(const string &val);

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

    uint64_t operator()(const string_view& x) const {
        // return fnv1a((const uint8_t*) x.data(), x.size());
        return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
    }

    uint64_t operator()(const string& x) const {
        // return fnv1a((const uint8_t*) x.c_str(), x.length());
        return MurmurHash64A((const uint8_t*) x.c_str(), x.length(), 0xFF);
    }

    uint64_t operator()(Item& x) const {        
        if(x.hash != 0){
            return x.hash;
        }

        uint16_t t_id = x.t_id;
        uint64_t hash; 
        switch(t_id) {
            case T_ID_BOOL:
                hash = CREHash::operator()(item_get_bool(x)); break;
            case T_ID_INT:
                hash = CREHash::operator()(item_get_int(x)); break;
            case T_ID_FLOAT:
                hash = CREHash::operator()(item_get_float(x)); break;
            case T_ID_STR:
                hash = CREHash::operator()(item_get_string(x)); break;
            default:
                hash = uint64_t (-1);
        }

        x.hash = hash;
        return hash;
    }
};


#endif /* _CRE_HASH_H_ */
