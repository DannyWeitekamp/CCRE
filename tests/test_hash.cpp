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
#include "test_macros.h"



#include "../include/hash.h"
#include <chrono>


using namespace std;
using namespace chrono;




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
    uint64_t operator()(const string& s) const
    {
        return rapidhash(s.c_str(), s.length());
    }
};

class STDHash {
public:
    uint64_t operator()(const string& s) const
    {
        return string_hash(s);
    }
};

class FNV1AHash {
public:
    uint64_t operator()(const string& s) const
    {
        return fnv1a((const uint8_t*) s.c_str(), s.length());
    }
};



class XXHash {
public:
    uint64_t operator()(const string& s) const
    {
        return xxh::xxhash<64>((const uint8_t*) s.c_str(), s.length(), 0xFF);
    }
};

class MurmurHash {
public:
    uint64_t operator()(const string& s) const
    {
        return MurmurHash64A((const uint8_t*) s.c_str(), s.length(), 0xFF);
    }
};

class SipHash {
public:
    uint64_t operator()(const string& s) const
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
    UnicodeItem str_item1 = std::bit_cast<UnicodeItem>(to_item(s1));
    cout << (uint64_t) s1.data() << ", " << (uint64_t) str_item1.data << endl;
    UnicodeItem str_item2 = std::bit_cast<UnicodeItem>(to_item(s2));
    cout << (uint64_t) s2.data() << ", " << (uint64_t) str_item2.data << endl;
}




// Test for function1()
// You would need to write these even when using a framework
void bench_hash() {
    const string str1 = string("Hellooooooooooooo");
    const string str2 = string("Hello");
    
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

    time_it_n("copy words, mobydick\t",
        for (int i=0; i < words.size(); i++) { 
            new std::string(words[i]); 
        }, 100);

}

void test_fact_hash(){
    Fact* boop = make_fact(NULL, "A", 1);
    cout << CREHash{}(boop) << endl;
}

int main(void) {
    // Call all tests. Using a test framework would simplify this.
    // bench_hash();
    test_fact_hash();
}
