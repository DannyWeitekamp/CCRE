#include <cstdint>
#include <string>

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
