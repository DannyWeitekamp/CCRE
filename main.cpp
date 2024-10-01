// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <cstdint>
#include "types.h"
#include "state.h"
#include "unicode.h"

#include <chrono>
using namespace std;
using namespace std::chrono;




State* setup_declare(int N=100){
  State* state = new State(3*(N+1));
  for(int i=0; i < N; i++){
    int k = i % 50;
    FactView fact = empty_fact(state, 2);

    fact.set(0, to_string(k));
    fact.set(1, k);

    declare(state, fact);  
  }  
  return state;
}

State* bench_declare(int N=10000){
  auto start = high_resolution_clock::now();

  State* state = setup_declare(N);
  auto stop = high_resolution_clock::now();  
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "declare(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;
  return state;
}

State* test_full_retract(int N=100, bool reverse=false){
  State* state = setup_declare(N);

  // cout << "REVERSE=" << reverse << endl;
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point stop;
  if(reverse){
    vector<FactHeader*> facts;  
    for (auto it = state->begin(); it != state->end(); ++it) {
      facts.push_back(&(*it));
    }
    start = high_resolution_clock::now();
    for (auto it = facts.rbegin(); it != facts.rend(); ++it) { 
      retract(state, (*it)->f_id);
    }
    stop = high_resolution_clock::now();
  }else{
    start = high_resolution_clock::now();
    for (auto it = state->begin(); it != state->end(); ++it) {
      // cout << "f_id: " << (&(*it))->f_id << endl;
      // state->print_layout();
      retract(state, (&(*it))->f_id);
    }
    stop = high_resolution_clock::now();
  }

  auto duration = duration_cast<microseconds>(stop - start);
  cout << "retract(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;

  // state->print_layout();
  return state;
}

State* test_retract(){
  State* state = setup_declare(10);

  // Forward retractions;
  retract(state, 4);
  retract(state, 7);
  retract(state, 10);

  // Reverse retractions;
  retract(state, 22);
  retract(state, 19);
  retract(state, 16);

  // Hole Filling retraction; 
  retract(state, 13);

  // Fill a few
  empty_fact(state, 2);
  empty_fact(state, 2);
  empty_fact(state, 2);

  // Fill one that doesn't fit in the current hole
  empty_fact(state, 100);

  state = setup_declare(10);
  // Get back to state w/ hole ;
  retract(state, 4);
  retract(state, 7);
  retract(state, 10);
  retract(state, 22);
  retract(state, 19);
  retract(state, 16);

  cout << "---------------" << endl;
  state->print_layout();

  // Fill one of the holes, mostly
  empty_fact(state, 4);
  empty_fact(state, 2);
  state->print_layout();

  // Fill other hole
  empty_fact(state, 4);
  state->print_layout();

  // Too big for hole
  empty_fact(state, 10);
  state->print_layout();

  // Just right
  empty_fact(state, 0);
  state->print_layout();

  // One more
  empty_fact(state, 9);
  state->print_layout();

  // Fill the last block exactly
  empty_fact(state, 3);
  state->print_layout();

  // Final one
  empty_fact(state, 9);
  state->print_layout();
  

  return state;
}

State* test_random_declare_retracts(int N=10000, int M=10, double hole_prop=.1){
  State* state = new State();

  vector<FactView> facts = {};
  for(int i=0; i < N; i++){
    int size = rand() % M;
    FactView fact = empty_fact(state, size);
    facts.push_back(fact);
  }

  // Randomly retract some so there are holes;
  for(int i=0; i < int(N * hole_prop); i++){
    size_t ind = rand() % facts.size();
    FactView fact = facts[ind];
    retract(state, fact);
    facts[ind] = facts[facts.size()-1];
    facts.pop_back();
  }

  auto start = high_resolution_clock::now();
  for(int i=0; i < N; i++){  
    int is_dec = rand() % 2;
    if(is_dec){
      int size = rand() % M;
      FactView fact = empty_fact(state, size);
      facts.push_back(fact);
    }else{
      size_t ind = rand() % facts.size();
      FactView fact = facts[ind];

      retract(state, fact);   
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
  for (auto it = state->begin(); it != state->end(); ++it) {
    sum += (&(*it))->f_id;
    //   // cout << "f_id: " << (&(*it))->f_id << endl;
    //   // state->print_layout();
    //   retract(state, (&(*it))->f_id);
    // }
    // sum += state->data[facts[i].f_id].f_id;
  }
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "  traverse allocs(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;
  // state->print_layout();
  return state;
}

State* test_random_malloc_free(int N=10000, int M=10, double hole_prop=.1){
  State* state = new State();

  vector<int64_t*> facts = {};
  for(int i=0; i < N; i++){
    int size = rand() % M;
    int64_t* fact = (int64_t*) malloc(sizeof(Block)*(size+1));
    memset(fact, 0, sizeof(Block)*(size+1));
    facts.push_back(fact);
  }

  // Randomly retract some so there are holes;
  for(int i=0; i < int(N * hole_prop); i++){
    size_t ind = rand() % facts.size();
    void* fact = facts[ind];
    free(fact);
    // retract(state, fact);
    facts[ind] = facts[facts.size()-1];
    facts.pop_back();
  }

  auto start = high_resolution_clock::now();
  for(int i=0; i < N; i++){  
    int is_dec = rand() % 2;
    if(is_dec){
      int size = rand() % M;
      int64_t* fact = (int64_t*) malloc(sizeof(Block)*(size+1));
      memset(fact, 0, sizeof(Block)*(size+1));
      facts.push_back(fact);
    }else{
      size_t ind = rand() % facts.size();
      void* fact = facts[ind];
      free(fact);
      // retract(state, fact);   
      facts[ind] = facts[facts.size()-1];
      facts.pop_back();
    }
  }
  auto stop = high_resolution_clock::now();  
  auto duration = duration_cast<microseconds>(stop - start);
  cout << "rand malloc-free(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;

  start = high_resolution_clock::now();
  int64_t sum = 0;
  for(uint i=0; i < facts.size(); i++){
    sum += *facts[i];
  }
  stop = high_resolution_clock::now();
  duration = duration_cast<microseconds>(stop - start);
  cout << "  traverse allocs(" << N << "): " << duration.count() / 1000.0 << "ms" << endl;


  // state->print_layout();
  return state;
}


int main() {
  CRE_Context* default_context = new CRE_Context("default");
  cout << "Context:" << default_context->name << "\nWith Types:\n";

  Type* point_type = define_type(default_context, "Point", 
    {{"x", cre_float}, {"y", cre_float}}
  );

  for (auto & t : default_context->types) {
    cout << "\t" << t << "\n";
  }

  cout << "MOOP:";
  cout << token_get_int(to_token(2)) << "\n";
  cout << token_get_float(to_token(2.0)) << "\n";
  cout << token_get_string(to_token("ABC")) << "\n";

  bench_declare();
  // test_full_retract();
  // State* state = setup_declare();
  // state->print_layout();
  // State* state = test_retract();
  // test_full_retract(10000, true);
  // test_full_retract(10000, false);
  // test_full_retract(10000, true);
  // test_full_retract(10000, false);

  // State* state = test_full_retract(100, true);
  // test_retract();
  test_random_malloc_free(50000);
  test_random_declare_retracts(50000);
  

  // cout << "------" << endl;
  // FactHeader* fact = empty_fact(state, 2);
  // fact->set(0, "Q");
  // fact->set(1, 2);


  // declare(state, fact);
  // state->print_layout();
  // return 0;
  // cout << "BEGIN: " << bit_cast<uint64_t>(state->data) << endl;
  // cout << "BEGIN: " << bit_cast<uint64_t>(state->begin()) << endl;
  // cout << "END: " << bit_cast<uint64_t>(state->end()) << endl;
  // for(auto fact = state.begin())
  // int i = 0;
  // for (auto it = state->begin(); it != state->end(); ++it) {
  //   uint32_t f_id = it->f_id;
  //   std::cout << fact_to_string(*it) << "f_id: " << f_id << " " <<fact_to_string(*get(state, it->f_id)) << endl;
  // }
      


  

  // cout << intern_refcount("1") << "\n";
  // cout << intern_refcount("2") << "\n";
  // cout << intern_refcount("3") << "\n";
  

  return 0;
}



/* Thinking Thinking Thinking

Advantages of having facts owned by their state:


*/
