// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <cstdint>
#include <cstring>

#include "include/types.h"
#include "include/context.h"
#include "include/item.h"
#include "include/fact.h"
#include "include/factset.h"
// #include "include/unicode.h"
#include "include/cre_obj.h"

#include <chrono>
using namespace std;
using namespace std::chrono;


FactSet* setup_declare(int N=100){
  FactSet* fs = new FactSet(3*(N+1));
  for(int i=0; i < N; i++){
    int k = i % 50;
    Fact* fact = empty_untyped_fact(2);

    fact->set(0, to_string(k));
    fact->set(1, k);

    declare(fs, fact);  
  }  
  return fs;
}

FactSet* bench_declare(int N=10000){
  auto start = high_resolution_clock::now();

  FactSet* fs = setup_declare(N);
  auto stop = high_resolution_clock::now();  
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "declare(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;
  return fs;
}

FactSet* test_full_retract(int N=100, bool reverse=false){
  FactSet* fs = setup_declare(N);

  // cout << "REVERSE=" << reverse << endl;
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point stop;
  if(reverse){
    vector<Fact*> facts; 
    // cout << "HERE"<< (void*)(&*fs->begin()) << "," << (void*) (&*fs->end()) << endl; 
    // cout << "HERE"<< (void*)(&*fs->begin()) << "," << (void*) (&*fs->end()) << endl; 
    for (auto it = fs->begin(); it != fs->end(); ++it) {
      Fact* fact = *it;
      // cout << fact_to_string(fact) << endl;
      facts.push_back(fact);
    }
    start = high_resolution_clock::now();
    for (auto it = facts.rbegin(); it != facts.rend(); ++it) { 
      retract(fs, (*it));
    }
    stop = high_resolution_clock::now();
  }else{
    start = high_resolution_clock::now();
    for (auto it = fs->begin(); it != fs->end(); ++it) {
      // cout << "f_id: " << (&(*it))->f_id << endl;
      // fs->print_layout();
      retract(fs, (*it));
    }
    stop = high_resolution_clock::now();
  }

  auto duration = duration_cast<microseconds>(stop - start);
  cout << "retract(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;

  // fs->print_layout();
  return fs;
}

// FactSet* test_retract(){
//   FactSet* fs = setup_declare(10);

//   // Forward retractions;
//   retract(fs, 4);
//   retract(fs, 7);
//   retract(fs, 10);

//   // Reverse retractions;
//   retract(fs, 22);
//   retract(fs, 19);
//   retract(fs, 16);

//   // Hole Filling retraction; 
//   retract(fs, 13);

//   // Fill a few
//   empty_fact(fs, 2);
//   empty_fact(fs, 2);
//   empty_fact(fs, 2);

//   // Fill one that doesn't fit in the current hole
//   empty_fact(fs, 100);

//   fs = setup_declare(10);
//   // Get back to fs w/ hole ;
//   retract(fs, 4);
//   retract(fs, 7);
//   retract(fs, 10);
//   retract(fs, 22);
//   retract(fs, 19);
//   retract(fs, 16);

//   cout << "---------------" << endl;
//   fs->print_layout();

//   // Fill one of the holes, mostly
//   empty_fact(fs, 4);
//   empty_fact(fs, 2);
//   fs->print_layout();

//   // Fill other hole
//   empty_fact(fs, 4);
//   fs->print_layout();

//   // Too big for hole
//   empty_fact(fs, 10);
//   fs->print_layout();

//   // Just right
//   empty_fact(fs, 0);
//   fs->print_layout();

//   // One more
//   empty_fact(fs, 9);
//   fs->print_layout();

//   // Fill the last block exactly
//   empty_fact(fs, 3);
//   fs->print_layout();

//   // Final one
//   empty_fact(fs, 9);
//   fs->print_layout();
  

//   return fs;
// }

FactSet* test_random_declare_retracts(int N=10000, int M=10, double hole_prop=.1){
  FactSet* fs = new FactSet();

  vector<Fact*> facts = {};
  for(int i=0; i < N; i++){
    int size = rand() % M;
    Fact* fact = empty_untyped_fact(size);
    declare(fs, fact);
    facts.push_back(fact);
  }

  // Randomly retract some so there are holes;
  for(int i=0; i < int(N * hole_prop); i++){
    size_t ind = rand() % facts.size();
    Fact* fact = facts[ind];
    retract(fs, fact);
    facts[ind] = facts[facts.size()-1];
    facts.pop_back();
  }

  auto start = high_resolution_clock::now();
  for(int i=0; i < N; i++){  
    int is_dec = rand() % 2;
    if(is_dec){
      int size = rand() % M;
      Fact* fact = empty_untyped_fact(size);
      declare(fs, fact);
      facts.push_back(fact);
    }else{
      size_t ind = rand() % facts.size();
      Fact* fact = facts[ind];

      retract(fs, fact);   
      facts[ind] = facts[facts.size()-1];
      facts.pop_back();
    }
  }
  auto stop = high_resolution_clock::now();  
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "rand declare+retract(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;

  start = high_resolution_clock::now();
  int64_t sum = 0;
  // for(int i=0; i < facts.size(); i++){
  for (auto it = fs->begin(); it != fs->end(); ++it) {
    sum += (*it)->f_id;
    //   // cout << "f_id: " << (&(*it))->f_id << endl;
    //   // fs->print_layout();
    //   retract(fs, (&(*it))->f_id);
    // }
    // sum += fs->data[facts[i].f_id].f_id;
  }
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "  traverse allocs(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;
  
  return fs;
}

// FactSet* test_random_malloc_free(int N=10000, int M=10, double hole_prop=.1){
//   FactSet* fs = new FactSet();

//   vector<int64_t*> facts = {};
//   for(int i=0; i < N; i++){
//     int size = rand() % M;
//     int64_t* fact = (int64_t*) malloc(sizeof(Block)*(size+1));
//     memset(fact, 0, sizeof(Block)*(size+1));
//     facts.push_back(fact);
//   }

//   // Randomly retract some so there are holes;
//   for(int i=0; i < int(N * hole_prop); i++){
//     size_t ind = rand() % facts.size();
//     void* fact = facts[ind];
//     free(fact);
//     // retract(fs, fact);
//     facts[ind] = facts[facts.size()-1];
//     facts.pop_back();
//   }

//   auto start = high_resolution_clock::now();
//   for(int i=0; i < N; i++){  
//     int is_dec = rand() % 2;
//     if(is_dec){
//       int size = rand() % M;
//       int64_t* fact = (int64_t*) malloc(sizeof(Block)*(size+1));
//       memset(fact, 0, sizeof(Block)*(size+1));
//       facts.push_back(fact);
//     }else{
//       size_t ind = rand() % facts.size();
//       void* fact = facts[ind];
//       free(fact);
//       // retract(fs, fact);   
//       facts[ind] = facts[facts.size()-1];
//       facts.pop_back();
//     }
//   }
//   auto stop = high_resolution_clock::now();  
//   auto duration = duration_cast<microseconds>(stop - start);
//   cout << "rand malloc-free(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;

//   start = high_resolution_clock::now();
//   int64_t sum = 0;
//   for(uint i=0; i < facts.size(); i++){
//     sum += *facts[i];
//   }
//   stop = high_resolution_clock::now();
//   duration = duration_cast<microseconds>(stop - start);
//   cout << "  traverse allocs(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;


//   // fs->print_layout();
//   return fs;
// }


#define time_it(descr, a) \
    do { \
        auto start = high_resolution_clock::now(); \
        a; \
        auto stop = high_resolution_clock::now(); \
        auto duration = duration_cast<microseconds>(stop - start); \
        cout << descr << ": " << fixed << duration.count() / (1000.0) << "ms" << endl; \
    } while (0)

#define time_it_n(descr, a, N) \
    do { \
    auto start = high_resolution_clock::now(); \
    for(int i=0; i < N; i++){ \
        a; \
    } \
    auto stop = high_resolution_clock::now(); \
    auto duration = duration_cast<microseconds>(stop - start); \
    cout << descr << ": " << fixed << duration.count() / (1000.0 * N) << "ms" << endl; \
    } while (0)






int main() {
  CRE_Context* default_context = new CRE_Context("default");
  cout << "Context:" << default_context->name << "\nWith Types:\n";

  FactType* point_type = define_fact("Point", 
    {{"x", cre_float}, {"y", cre_float}}
  );

  for (auto & t : default_context->types) {
    cout << "\t" << t << "\n";
  }

  cout << "MOOP:";
  cout << item_get_int(to_item(2)) << "\n";
  cout << item_get_float(to_item(2.0)) << "\n";

  // std::string abc = "ABC";
  // cout << abc;
  // cout << item_get_string(to_item(string_view(abc))) << "\n";
  // cout << abc;
  cout << item_get_string(to_item("ABC")) << "\n";

  auto obj = new CRE_Obj();
  cout << "RECOUNT:" << obj->get_refcount() << endl;
  CRE_incref(obj);
  cout << "RECOUNT:" << obj->get_refcount() << endl;
  CRE_decref(obj);
  cout << "RECOUNT:" << obj->get_refcount() << endl;
  CRE_decref(obj);



  

  // cout << fact_to_string(fact) << endl;

  time_it_n("new facts",
    for(int i=0; i < 1000; i++){
      auto fact = empty_untyped_fact(3);
      fact->set(0, i);
      // fact->set(1, "A");
      // fact->set(2, true);    
    }
  ,1000);

  time_it_n("incref facts",
    auto fact = empty_untyped_fact(3);
    for(int i=0; i < 1000; i++){
      CRE_incref(fact);
    }
    
  ,1000);

  // cout << "RECOUNT:" << obj->get_refcount() << endl;

  bench_declare();

  FactSet* fs;
  // test_full_retract();
  // fs = setup_declare();
  // // fs->print_layout();
  // fs = test_retract();
  test_full_retract(10000, true);
  // test_full_retract(10000, false);
  // test_full_retract(10000, true);
  // test_full_retract(10000, false);

  // FactSet* fs = test_full_retract(100, true);
  // test_retract();
  // test_random_malloc_free(50000);
  test_random_declare_retracts(50000);

  cout << setup_declare() << endl;
  

  // cout << "------" << endl;
  // Fact* fact = empty_fact(fs, 2);
  // fact->set(0, "Q");
  // fact->set(1, 2);


  // declare(fs, fact);
  // fs->print_layout();
  // return 0;
  // cout << "BEGIN: " << bit_cast<uint64_t>(fs->data) << endl;
  // cout << "BEGIN: " << bit_cast<uint64_t>(fs->begin()) << endl;
  // cout << "END: " << bit_cast<uint64_t>(fs->end()) << endl;
  // for(auto fact = fs.begin())
  // int i = 0;
  // for (auto it = fs->begin(); it != fs->end(); ++it) {
  //   uint32_t f_id = it->f_id;
  //   std::cout << fact_to_string(*it) << "f_id: " << f_id << " " <<fact_to_string(*get(fs, it->f_id)) << endl;
  // }
      


  

  // cout << intern_refcount("1") << "\n";
  // cout << intern_refcount("2") << "\n";
  // cout << intern_refcount("3") << "\n";
  

  return 0;
}



/* Thinking Thinking Thinking

Advantages of having facts owned by their fs:


*/
