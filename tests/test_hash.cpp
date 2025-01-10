#include <functional>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <span>
#include <sstream>

#define XXH_INLINE_ALL 1


#include "../include/unordered_dense.h"
#include "../include/rapidhash.h"
#include "../include/xxhash.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "test_macros.h"



#include "../include/hash.h"


using std::cout;
using std::endl;
using std::string;




std::vector<std::string> readFileWordByWord(const std::string& filename) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return words;
    }

    std::string word;
    while (file >> word) {
        words.push_back(word);
    }

    file.close();
    return words;
}


std::hash<std::string> string_hash;

class FastHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return rapidhash(s.c_str(), s.length());
    }
};

class STDHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return string_hash(s);
    }
};

class FNV1AHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return fnv1a((const uint8_t*) s.c_str(), s.length());
    }
};



class XXHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return xxh::xxhash<64>((const uint8_t*) s.c_str(), s.length(), 0xFF);
    }
};

class MurmurHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return MurmurHash64A((const uint8_t*) s.c_str(), s.length(), 0xFF);
    }
};

class SipHash {
public:
    uint64_t operator()(const std::string& s) const
    {
        return siphash24(
            siphash_k0_default,
            siphash_k1_default,
            (const uint8_t*) s.c_str(), s.length());
    }
};





#define insert_n_read(map) \
    do { \
    for (int i=0; i < words.size(); i++) { \
        map[words[i]] = i; \
    }\
    uint64_t x = 0; \
    for (int i=0; i < words.size(); i++) { \
        x += map[words[i]]; \
    }\
    } while (0)

#define intern_words() \
    do { \
    for (int i=0; i < words.size(); i++) { \
        intern(words[i]]); \
    }\
    } while (0)



template<int left = 0, int right = 0, typename T>
constexpr auto slice(T&& container)
{
    if constexpr (right > 0)
    {
        return std::span(begin(std::forward<T>(container))+left, begin(std::forward<T>(container))+right);
    }
    else
    {
        return std::span(begin(std::forward<T>(container))+left, end(std::forward<T>(container))+right);
    }
}


void test_intern(){
    std::string s1 = "A";
    std::string s2 = "A";
    UnicodeItem str_item1 = std::bit_cast<UnicodeItem>(Item(s1));
    cout << (uint64_t) s1.data() << ", " << (uint64_t) str_item1.data << endl;
    UnicodeItem str_item2 = std::bit_cast<UnicodeItem>(Item(s2));
    cout << (uint64_t) s2.data() << ", " << (uint64_t) str_item2.data << endl;
}




// Test for function1()
// You would need to write these even when using a framework
void bench_hash() {
    const std::string str1 = std::string("Hellooooooooooooo");
    const std::string str2 = std::string("Hello");
    
    cout << "Hash of str1 as string: " << hash_string(str1) << endl;
    cout << "Hash of str2 as string: " << hash_string(str2) << endl;
    cout << "Hash of str1 as string: " << string_hash(str1) << endl;
    cout << "Hash of str2 as string: " << string_hash(str2) << endl;

    time_it_n("std::string_hash(long)\t", string_hash(str1), 10000);
    time_it_n("std::string_hash(short)\t", string_hash(str2), 10000);

    time_it_n("hash_string(long)\t", hash_string(str1), 10000);
    time_it_n("hash_string(short)\t", hash_string(str2), 10000);

    auto ank_fast_map = ankerl::unordered_dense::map<string, int, FastHash>();
    auto ank_std_map = ankerl::unordered_dense::map<string, int, STDHash>();
    auto ank_fnv_map = ankerl::unordered_dense::map<string, int, FNV1AHash>();
    auto ank_xxh_map = ankerl::unordered_dense::map<string, int, XXHash>();
    auto ank_mur_map = ankerl::unordered_dense::map<string, int, MurmurHash>();
    auto ank_sip_map = ankerl::unordered_dense::map<string, int, SipHash>();

    auto uord_fast_map = std::unordered_map<string, int, FastHash>();
    auto uord_std_map = std::unordered_map<string, int, STDHash>();
    auto uord_fnv_map = std::unordered_map<string, int, FNV1AHash>();
    auto uord_xxh_map = std::unordered_map<string, int, XXHash>();
    auto uord_mur_map = std::unordered_map<string, int, MurmurHash>();
    auto uord_sip_map = std::unordered_map<string, int, SipHash>();

    std::string filename = "../data/mobydick.txt";
    std::vector<std::string> words = readFileWordByWord(filename);
    std::cout << "# Words in the file:" << words.size() << std::endl;

    words = std::vector<std::string>(words.begin(), words.begin()+10000);
    // words = slice<0,1000>(words);

    // for (const auto& word : words) {
    time_it_n("ank_fast_map, mobydick\t", insert_n_read(ank_fast_map), 10);
    time_it_n("ank_std_map, mobydick\t", insert_n_read(ank_std_map), 10);
    time_it_n("ank_fnv_map, mobydick\t", insert_n_read(ank_fnv_map), 10);
    time_it_n("ank_xxh_map, mobydick\t", insert_n_read(ank_xxh_map), 10);
    time_it_n("ank_mur_map, mobydick\t", insert_n_read(ank_mur_map), 10);
    time_it_n("ank_sip_map, mobydick\t", insert_n_read(ank_sip_map), 10);
    cout << endl;
    time_it_n("uord_fast_map, mobydick\t", insert_n_read(uord_fast_map), 10);
    time_it_n("uord_std_map, mobydick\t", insert_n_read(uord_std_map), 10);
    time_it_n("uord_fnv_map, mobydick\t", insert_n_read(uord_fnv_map), 10);
    time_it_n("uord_xxh_map, mobydick\t", insert_n_read(uord_xxh_map), 10);
    time_it_n("uord_mur_map, mobydick\t", insert_n_read(uord_mur_map), 10);
    time_it_n("uord_sip_map, mobydick\t", insert_n_read(uord_sip_map), 10);
    cout << endl;

    time_it_n("intern_words, mobydick\t",
        for (int i=0; i < words.size(); i++) { 
            intern(words[i]); 
        }, 100);

    time_it_n("intern_words w/ context, mobydick\t",
        CRE_Context* const context = current_context;
        for (int i=0; i < words.size(); i++) { 
            context->intern(words[i]); 
        }, 100);

    time_it_n("copy words, mobydick\t",
        for (int i=0; i < words.size(); i++) { 
            new std::string(words[i]); 
        }, 100);

}

std::string bits_to_string(uint64_t x) {
    std::string result;
    for (int i = 63; i >= 0; i--) {
        result += ((x >> i) & 1) ? '1' : '0';
        if (i % 8 == 0 && i != 0) result += ' ';
    }
    return result;
}

std::string bits_to_string(const std::vector<uint8_t>& x) {
    std::string result;
    for (int i = 0; i < x.size(); i++) {
        for (int j = 7; j >= 0; --j) {
            result += ((x[i] >> j) & 1) ? '1' : '0';
            if (j % 8 == 0 && j != 0) result += ' ';
        }
    }
    return result;
}

void test_factset_hash(int N=1000){
    
    cout << IntHash(0) << endl;
    cout << IntHash(1) << endl;
    cout << IntHash(1) << endl;
    cout << IntHash(2) << endl;
    cout << IntHash(3) << endl;
    cout << bits_to_string(FNV_BASIS) << endl;
    cout << bits_to_string(FNV_BASIS*FNV_BASIS) << endl;
    cout << bits_to_string(int64_t(FNV_BASIS) * int64_t(FNV_BASIS)) << endl;
    cout << bits_to_string(uint64_t(FNV_BASIS) * uint64_t(FNV_BASIS))<< endl;

    //  --- Check that simple facts have random-ish hashes ---
    std::vector<int> ones(64, 0);
    for(int i=0; i < N; i++){
        ref<Fact> fact = make_fact(nullptr, Item(i), Item(std::to_string(i)), Item(i%2));

        uint64_t hash = CREHash{}(fact);
        for (int j = 0; j < 64; j++) {
            ones[j] += (hash >> j) & 1;
        }   
    }
    double avg_bias = 0;
    for(int i=0; i < 64; i++){
        double one_prop = double(ones[i]) / N;
        avg_bias += std::abs(.5-one_prop);
    }
    avg_bias /= 64;
    // cout << "avg_bias" << avg_bias << endl;
    IS_TRUE(avg_bias < .05);


    // --- Check that FactSets have random-ish hashes --
    int byte_width = 24;
    double total_duration;
    // total_duration = 0;
    ones = std::vector(byte_width*8, 0);
    for(int k=0; k < N; k++){
        ref<FactSet> fs = new FactSet();
        for(int i=0; i < N; i++){
            ref<Fact> fact = make_fact(nullptr, Item(i*k), Item(std::to_string(i*k)), Item(i%2));
            fs->declare(fact);
        }

        auto start = high_resolution_clock::now();        
        std::vector<uint8_t> hash = fs->long_hash_bytes(byte_width);//CREHash{}(fact);
        auto stop = high_resolution_clock::now();        
        auto dur = duration_cast<microseconds>(stop-start);
        // cout << dur.count() / 1000.0 << endl;
        total_duration += dur.count() / 1000.0;

        for (int j = 0; j < byte_width*8; j++) {
            ones[j] += (hash[j/8] >> (j%8)) & 1;
        }
    }
    avg_bias = 0;
    for(int i=0; i < byte_width*8; i++){
        double one_prop = double(ones[i]) / N;
        avg_bias += std::abs(.5-one_prop);
        // cout << "bit=" << i << " one_prop=" << one_prop << endl;
    }
    avg_bias /= byte_width*8;
    IS_TRUE(avg_bias < .05);
    cout << "avg_bias:" << avg_bias << endl;
    cout << "avg_time:" <<  total_duration / N << " ms" << endl;
    

    // --- Check that the order of the elements doesn't effect the hash ---
    ref<FactSet> fs_fwd = new FactSet();
    for(int i=0; i < 10; i++){
        ref<Fact> fact = make_fact(nullptr, Item(i), Item(std::to_string(i)), Item(i%2));
        cout << fact->hash << endl;
        fs_fwd->declare(fact);
    }
    std::vector<uint8_t> fwd_hash = fs_fwd->long_hash_bytes(byte_width);
    cout << " -- -- " << endl;
    ref<FactSet> fs_rev = new FactSet();
    for(int i=10-1; i > -1; --i){
        ref<Fact> fact = make_fact(nullptr, Item(i), Item(std::to_string(i)), Item(i%2));
        cout << fact->hash << endl;
        fs_rev->declare(fact);
    }
    std::vector<uint8_t> rev_hash = fs_rev->long_hash_bytes(byte_width);

    cout << fs_fwd << endl;
    cout << fs_rev << endl;

    cout << bits_to_string(fwd_hash) << endl;
    cout << bits_to_string(rev_hash) << endl;

    IS_TRUE(fwd_hash == rev_hash);

    cout << fs_rev->long_hash_string(byte_width) << endl;


}

int main(void) {
    // Call all tests. Using a test framework would simplify this.
    // bench_hash();
    test_factset_hash();
}
