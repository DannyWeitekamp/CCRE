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
    Fact* fact = empty_fact(state, 2);

    fact->set(0, to_string(k));
    fact->set(1, k);

    declare(state, fact);  
  }  
  return state;
}

State* bench_declare(int N=10000){
  auto start = high_resolution_clock::now();

  State* state = setup_declare(N);
  auto stop = high_resolution_clock::now();  
  auto duration = duration_cast<microseconds>(stop - start);
  cout << duration.count() / 1000.0 << "ms" << endl;
  return state;
}

State* test_full_retract(int N=100, bool reverse=false){
  State* state = setup_declare(N);

  cout << "REVERSE=" << reverse << endl;
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point stop;
  if(reverse){
    vector<Fact*> facts;  
    for (auto it = state->begin(); it != state->end(); ++it) {
      facts.push_back(&(*it));
    }
    start = high_resolution_clock::now();
    for (auto it = facts.rbegin(); it != facts.rend(); ++it) { 
      retract(state, *it);
    }
    stop = high_resolution_clock::now();
  }else{
    start = high_resolution_clock::now();
    for (auto it = state->begin(); it != state->end(); ++it) {
      retract(state, &(*it));
    }
    stop = high_resolution_clock::now();
  }

  auto duration = duration_cast<microseconds>(stop - start);
  cout << duration.count() / 1000.0 << "ms" << endl;

  state->print_layout();
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

  // state->print_layout();
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

  state->print_layout();

  // Fill one of the holes, mostly
  empty_fact(state, 3);
  empty_fact(state, 2);

  state->print_layout();

  // Fill other hole
  empty_fact(state, 6);

  state->print_layout();

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

  // bench_declare();
  // State* state = setup_declare();
  // state->print_layout();
  State* state = test_retract();
  // test_retract(10000, false);
  // State* state = test_full_retract(100, true);

  // cout << "------" << endl;
  // Fact* fact = empty_fact(state, 2);
  // fact->set(0, "Q");
  // fact->set(1, 2);


  // declare(state, fact);
  state->print_layout();
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
