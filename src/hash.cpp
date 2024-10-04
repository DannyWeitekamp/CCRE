/* 
Implementation of various hash utilities used in CRE. Hash functions
 are hard-coded to ensure consistency between processes and systems
 which is important so that states always have the same hash between 
 runs. We explicitly do not use std::hash since it does not have these 
 gaurentees. Most hash functions here are ripped from CPython because 
 they are implemented with cross platform consistency in minds,
 and are relatively fast. Hash values are not guarenteed to be the
 same as CPython, and in many cases cyptographic gaurentees are abondoned
 for improved performance. 
*/


#include <cstdint>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
#include <string>
#include "../include/rapidhash.h"
#include "../include/hash.h"
#include "../include/item.h"
#include "../include/types.h"

using namespace std;

#ifdef __APPLE__
#  include <libkern/OSByteOrder.h>
#elif defined(HAVE_LE64TOH) && defined(HAVE_ENDIAN_H)
#  include <endian.h>
#elif defined(HAVE_LE64TOH) && defined(HAVE_SYS_ENDIAN_H)
#  include <sys/endian.h>
#endif


const uint64_t _PyHASH_XXPRIME_1 = 11400714785074694791ULL;
const uint64_t _PyHASH_XXPRIME_2 = 14029467366897019727ULL;
const uint64_t _PyHASH_XXPRIME_5 = 2870177450012600261ULL;

// Use constant  
const uint64_t siphash_k0_default = 7823388449531244853ULL;
const uint64_t siphash_k1_default = 2452911659039219235ULL;

#define PyHASH_MODULUS (((size_t)1 << _PyHASH_BITS) - 1)
#define _PyHASH_INF 314159

// NOTE: Python uses pointer hashes for NaN, to avoid pileup 
//  here we just pick an arbitrary value; 
#define _ArbHASH_NaN 567835

#define ROTATE(x, b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )


#define HALF_ROUND(a,b,c,d,s,t)     \
    a += b; c += d;                 \
    b = ROTATE(b, s) ^ a;           \
    d = ROTATE(d, t) ^ c;           \
    a = ROTATE(a, 32);

#define SINGLE_ROUND(v0,v1,v2,v3)   \
    HALF_ROUND(v0,v1,v2,v3,13,16);  \
    HALF_ROUND(v2,v1,v0,v3,17,21);

#define DOUBLE_ROUND(v0,v1,v2,v3)   \
    SINGLE_ROUND(v0,v1,v2,v3);      \
    SINGLE_ROUND(v0,v1,v2,v3);


#if PY_LITTLE_ENDIAN
#  define _le64toh(x) ((uint64_t)(x))
#elif defined(__APPLE__)
#  define _le64toh(x) OSSwapLittleToHostInt64(x)
#elif defined(HAVE_LETOH64)
#  define _le64toh(x) le64toh(x)
#else
#  define _le64toh(x) (((uint64_t)(x) << 56) | \
                      (((uint64_t)(x) << 40) & 0xff000000000000ULL) | \
                      (((uint64_t)(x) << 24) & 0xff0000000000ULL) | \
                      (((uint64_t)(x) << 8)  & 0xff00000000ULL) | \
                      (((uint64_t)(x) >> 8)  & 0xff000000ULL) | \
                      (((uint64_t)(x) >> 24) & 0xff0000ULL) | \
                      (((uint64_t)(x) >> 40) & 0xff00ULL) | \
                      ((uint64_t)(x)  >> 56))
#endif


#define _PyHASH_MULTIPLIER 1000003UL  /* 0xf4243 */
#define _PyHASH_BITS 61
#define _PyHASH_MODULUS (((size_t)1 << _PyHASH_BITS) - 1)


// Essentially a copy of Python's siphash24 
//  (https://github.com/python/cpython/blob/7a178b760573dcb7d67138129baad27a24ab202d/Python/pyhash.c#L422)

uint64_t siphash24(uint64_t k0, uint64_t k1, const void* src, uint64_t src_sz) {
    uint64_t b = (uint64_t)src_sz << 56;
    const uint8_t *in = (const uint8_t*)src;

    uint64_t v0 = k0 ^ 0x736f6d6570736575ULL;
    uint64_t v1 = k1 ^ 0x646f72616e646f6dULL;
    uint64_t v2 = k0 ^ 0x6c7967656e657261ULL;
    uint64_t v3 = k1 ^ 0x7465646279746573ULL;

    uint64_t t;
    uint8_t *pt;
    uint64_t idx = 0;

    while (src_sz >= 8) {
        uint64_t mi;
        memcpy(&mi, in, sizeof(mi));
        mi = _le64toh(mi);
        idx += sizeof(mi);
        src_sz -= 8;
        v3 ^= mi;
        DOUBLE_ROUND(v0,v1,v2,v3);
        v0 ^= mi;
    }

    t = 0;
    pt = (uint8_t *)&t;
    switch (src_sz) {
        case 7: pt[6] = in[6]; [[fallthrough]];
        case 6: pt[5] = in[5]; [[fallthrough]];
        case 5: pt[4] = in[4]; [[fallthrough]];
        case 4: memcpy(pt, in, sizeof(uint32_t)); break;
        case 3: pt[2] = in[2]; [[fallthrough]];
        case 2: pt[1] = in[1]; [[fallthrough]];
        case 1: pt[0] = in[0]; break;
    }
    b |= _le64toh(t);

    v3 ^= b;
    DOUBLE_ROUND(v0,v1,v2,v3);
    v0 ^= b;
    v2 ^= 0xff;
    DOUBLE_ROUND(v0,v1,v2,v3);
    DOUBLE_ROUND(v0,v1,v2,v3);

    /* modified */
    t = (v0 ^ v1) ^ (v2 ^ v3);
    return t;
}

// uint64_t fnv1a(std::string const & text) {
uint64_t fnv1a(const uint8_t *val, uint64_t _len) {
    uint64_t constexpr fnv_prime = 1099511628211ULL;
    uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;
    
    uint64_t hash = fnv_offset_basis;
        
    for(uint64_t i=0; i < _len; i++){
        hash ^= val[i];
        hash *= fnv_prime;
    }

    return hash;
}

//-----------------------------------------------------------------------------
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

static inline uint64_t getblock ( const uint64_t * p )
{
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  return *p;
#else
  const uint8_t *c = (const uint8_t *)p;
  return (uint64_t)c[0] |
     (uint64_t)c[1] <<  8 |
     (uint64_t)c[2] << 16 |
     (uint64_t)c[3] << 24 |
     (uint64_t)c[4] << 32 |
     (uint64_t)c[5] << 40 |
     (uint64_t)c[6] << 48 |
     (uint64_t)c[7] << 56;
#endif
}


uint64_t MurmurHash64A ( const void * key, int len, uint64_t seed )
{
  const uint64_t m = 0xc6a4a7935bd1e995ULL;
  const int r = 47;

  uint64_t h = seed ^ (len * m);

  const uint64_t * data = (const uint64_t *)key;
  const uint64_t * end = data + (len/8);

  while(data != end)
  {
    uint64_t k = getblock(data++);

    k *= m; 
    k ^= k >> r; 
    k *= m; 
    
    h ^= k;
    h *= m; 
  }

  const unsigned char * data2 = (const unsigned char*)data;

  switch(len & 7)
  {
  case 7: h ^= uint64_t(data2[6]) << 48; [[fallthrough]];
  case 6: h ^= uint64_t(data2[5]) << 40; [[fallthrough]];
  case 5: h ^= uint64_t(data2[4]) << 32; [[fallthrough]];
  case 4: h ^= uint64_t(data2[3]) << 24; [[fallthrough]];
  case 3: h ^= uint64_t(data2[2]) << 16; [[fallthrough]];
  case 2: h ^= uint64_t(data2[1]) << 8; [[fallthrough]];
  case 1: h ^= uint64_t(data2[0]); 
          h *= m;
  };
 
  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return h;
} 

// inline uint64_t nonseq_fnv1a(const uint8_t *val, uint64_t _len) {
//     uint64_t constexpr fnv_prime = 1099511628211ULL;
//     uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;
    
//     uint64_t hash = fnv_offset_basis;

//     (const uint64_t*) val_u8 = (const uint64_t*) val_u8;

    
//     for(uint64_t i=0; i < _len; i+=8){
//         hash ^= ((uint64_t) val[i+0] | 
//                  (uint64_t) val[i+1] << 8 | 
//                  (uint64_t) val[i+2] << 16 | 
//                  (uint64_t) val[i+3] << 24 | 
//                  (uint64_t) val[i+4] << 32 | 
//                  (uint64_t) val[i+5] << 40 | 
//                  (uint64_t) val[i+6] << 48 | 
//                  (uint64_t) val[i+7] << 56
//         );
//     }

//     for(uint64_t j=0; j < 8; j++){
//         hash ^= val[i+j]<<(j<<3);
//     }
//     hash *= fnv_prime;

//     return hash;
// }



// #define hash_bytes(a, b) fnv1a(a, b)


// Essentially a copy of Python's HashDouble 
//  (https://github.com/python/cpython/blob/6810928927e4d12d9a5dd90e672afb096882b730/Python/pyhash.c#L87)
uint64_t _Py_HashDouble(double v){
    int e, sign;
    uint64_t x, y;
    double m;

    if (!isfinite(v)) {
        if (isinf(v)){
            return v > 0 ? _PyHASH_INF : -_PyHASH_INF;
        }else{
            // This line is different from CPython
            return _ArbHASH_NaN;
        }
    }

    m = frexp(v, &e);

    sign = 1;
    if (m < 0) {
        sign = -1;
        m = -m;
    }

    /* process 28 bits at a time;  this should work well both for binary
       and hexadecimal floating point. */
    x = 0;
    while (m) {
        x = ((x << 28) & _PyHASH_MODULUS) | x >> (_PyHASH_BITS - 28);
        m *= 268435456.0;  /* 2**28 */
        e -= 28;
        y = (uint64_t) m;  /* pull out integer part */
        m -= y;
        x += y;
        if (x >= _PyHASH_MODULUS)
            x -= _PyHASH_MODULUS;
    }

    /* adjust for the exponent;  first reduce it modulo _PyHASH_BITS */
    e = e >= 0 ? e % _PyHASH_BITS : _PyHASH_BITS-1-((-1-e) % _PyHASH_BITS);
    x = ((x << e) & _PyHASH_MODULUS) | x >> (_PyHASH_BITS - e);

    x = x * sign;
    if (x == (uint64_t) -1)
        x = (uint64_t) -2;
    return x;
}



uint64_t process_return(uint64_t val){
    if (val == (uint64_t) -1){
        val = (uint64_t) -2;
    }
    return val;
}

// Set a biggish hash-cutoff for using fast DJBX33A, instead of slow siphash 
// should speed things up appreciably, security sensitive things shouldn't
// be calling the no seed hash implementation anyway.
uint64_t _HASH_CUTOFF = 12;

inline uint64_t hash_bytes(const uint8_t *val, uint64_t _len){
    return rapidhash(val, _len);
    // if (_len == 0){
    //     return process_return(0);
    // }

    // uint64_t _hash;
    // // Use fast DJBX33A for small byte strings up to _HASH_CUTOFF
    // if (_len < _HASH_CUTOFF){
    //     _hash = 5381ULL;  /* DJBX33A starts with 5381 */
    //     for(uint64_t i=0; i < _len; i++){
    //         _hash = ((_hash << 5) + _hash) + val[i]; 
    //     }
    //     _hash ^= _len;

    // // For longer strings use siphash
    // }else{
    //     // uint64_t tmp = siphash24(siphash_k0_default,
    //     //                  siphash_k1_default,
    //     //                  val, _len);
    //     uint64_t tmp = fnv1a(val, _len);
    //     _hash = process_return(tmp);
    // }
    // return process_return(_hash);
}

uint64_t hash_string(const string &val){
    return hash_bytes((const uint8_t *) val.c_str(), val.length());
};


uint64_t _PyHASH_XXROTATE(uint64_t x){
    return (x << (uint64_t) 31) | (x >> (uint64_t) 33);
}

uint64_t accum_item_hash(uint64_t acc, uint64_t lane){
    if(lane == (uint64_t) -1){
        return 1546275796ULL;
    }
    acc += lane * _PyHASH_XXPRIME_2;
    acc = _PyHASH_XXROTATE(acc);
    acc *= _PyHASH_XXPRIME_1;
    return acc;
}




