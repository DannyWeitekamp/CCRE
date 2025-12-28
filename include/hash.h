#pragma once

#include <cstdint>
#include <string>
// #include <bit>
#include <unordered_map>
#include "../external/unordered_dense.h"
#include "../include/ref.h"
#include "../include/wref.h"
#include "../include/item.h"

namespace cre {

// NOTE: Do not import types.h here

// Forward Declarations
// struct Item;
struct Member;
class Fact;
struct Var;
struct FactView;
struct UintPair;
uint64_t hash_item(const Item& x);


extern const uint64_t _PyHASH_XXPRIME_1;
extern const uint64_t _PyHASH_XXPRIME_2;
extern const uint64_t _PyHASH_XXPRIME_5;
extern const uint64_t siphash_k0_default;
extern const uint64_t siphash_k1_default;

uint64_t constexpr FNV_PRIME = 1099511628211ULL;
uint64_t constexpr FNV_BASIS = 14695981039346656037ULL;


uint64_t _PyHASH_XXROTATE(uint64_t x);



uint64_t _Py_HashDouble_cre_impl(double v);
uint64_t hash_bytes(const uint8_t *val, uint64_t _len);
uint64_t fnv1a(const uint8_t *val, uint64_t _len);
uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed );
uint64_t hash_string(const std::string &val);

// Python's siphash24 
uint64_t siphash24(uint64_t k0, uint64_t k1, const void* src, uint64_t src_sz);


uint64_t accum_item_hash(uint64_t acc, uint64_t lane);
uint64_t process_return(uint64_t val);

uint64_t IntHash(const uint64_t& n);
std::string bytes_to_base64(std::vector<uint8_t> bytes);

struct CREHash {
    using is_transparent = void;
    using key_equal = std::equal_to<>;  // Pred to use

    uint64_t operator()([[maybe_unused]] std::nullptr_t x){
        return 0;
    }

    template <std::integral T>
    uint64_t operator()(const T& x) const {
        return IntHash(x);
    }

    template <std::floating_point T>
    uint64_t operator()(const T& x) const {
        return _Py_HashDouble_cre_impl(x);
    }



    uint64_t operator()(std::string_view x) const {
        // return fnv1a((const uint8_t*) x.data(), x.size());
        return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
    }

    uint64_t operator()(const std::string& x) const {
        // return fnv1a((const uint8_t*) x.data(), x.size());
        return CREHash{}(std::string_view(x));
    }

    // uint64_t operator()(const std::string& x) const {
    //     // return fnv1a((const uint8_t*) x.c_str(), x.length());
    //     return MurmurHash64A((const uint8_t*) x.c_str(), x.length(), 0xFF);
    // }
    uint64_t operator()(const char* x){
        return CREHash{}(std::string_view(x));
    }

    uint64_t operator()(const Item& x) const{
        return hash_item(x);
    }

    uint64_t operator()(const FactView& x) const;

    // Not Const
    uint64_t operator()(Fact* x);

    // Not Const
    uint64_t operator()(Var* x);

    template <class T>
    uint64_t operator()(const ref<T>& x){
        return CREHash{}(x.get());
    }

    template <class T>
    uint64_t operator()(const wref<T>& x){
        return CREHash{}(x.get());
    }

    // UintPair used in Vectorizer
    uint64_t operator()(const UintPair& x) const;
    

};


template<class T, class U>
// using HashMap = std::unordered_map<T, U, CREHash, std::equal_to<>>;
using HashMap = ankerl::unordered_dense::map<T, U, CREHash, std::equal_to<>>;

template<class T>
using HashSet = ankerl::unordered_dense::set<T, CREHash, std::equal_to<>>;


} // NAMESPACE_END(cre)

