// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <cstdint>
#include <cstring>

#include "../include/types.h"
#include "../include/hash.h"
#include "../include/context.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/intern.h"
// #include "../include/unicode.h"
#include "../include/cre_obj.h"
#include "../include/flattener.h"
#include "test_macros.h"

#include <chrono>
using namespace std;
using namespace std::chrono;


int loop_fact_set(FactSet* fs){
	volatile int x = 0;
	for (auto it = fs->begin(); it != fs->end(); ++it) {
	 	Fact* fact = (*it);
	 	x = x + fact->f_id;
      // cout << "f_id: " << (&(*it))->f_id << endl;
      // state->print_layout();
      // retract(state, (&(*it))->f_id);
    }
    return x;
}

FactSet* test_individual_build(size_t N){
	FactSet* fs = new FactSet();
	// Item str_item = to_item("A");
	std::vector<Item> items = {to_item(0), to_item("A"), to_item(false)};
	for(int i=0; i < N; i++){
		// std::vector<Item> items = {to_item(i), str_item, to_item(bool(i%2))};
		Fact* fact = new_fact(NULL, items.data(), items.size());
		declare(fs, fact);
	}
	return fs;
}

FactSet* test_unbuffered_build(size_t N){
	FactSetBuilder* fs_builder = new FactSetBuilder(N, 0);
	// cout << fs_builder->fact_set << endl;
	// Item str_item = to_item("A");
	std::vector<Item> items = {to_item(0), to_item("A"), to_item(false)};
	for(int i=0; i < N; i++){
		FactSetBuilder_add_fact(fs_builder, NULL, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder->fact_set;
}


FactSet* test_buffered_build(size_t N, size_t M){
	FactSetBuilder* fs_builder = new FactSetBuilder(N, N*(sizeof(Fact) + sizeof(Item)*M));
	// cout << fs_builder->fact_set << endl;
	// Item str_item = to_item("A");
	std::vector<Item> items = {to_item(0), to_item("A"), to_item(false)};
	for(int i=0; i < N; i++){
		FactSetBuilder_add_fact(fs_builder, NULL, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder->fact_set;
}

void test_build_from_json(){
	char* json_str;
	FactSet* fs;
	define_fact("Cat", 
	    {{"id", cre_str},
	     {"color", cre_str},
	     {"legs", cre_int},
	     {"frisky", cre_bool}
	 	}
	);
	cout << "HELLO2" << endl;

	fs = FactSet_from_json_file((char *) "../data/small_test_state.json");
	cout << "AFTER FS" << endl;
	cout << fs << endl;

	json_str = FactSet_to_json(fs);

	

	cout << "Written JSON str" << endl;
	cout << json_str << endl;
	
	time_it_n("from_json_file", fs=FactSet_from_json_file("../data/big_untyped_state.json"), 50);
	time_it_n("to_json", FactSet_to_json(fs), 50);

	fs = new FactSet();
	Fact* a = fs->add_fact(nullptr, {to_item(0), to_item("A"), to_item(false)});
	Fact* b = fs->add_fact(nullptr, {to_item(1), to_item("B"), to_item(true)});
	Fact* c = fs->add_fact(nullptr, {to_item(2), to_item("C"), to_item(false)});
	fs->add_fact(nullptr, {to_item(a), to_item(b), to_item(c)});

	cout << fs << endl;
	json_str = FactSet_to_json(fs);
	cout << json_str << endl;

	fs=FactSet_from_json(json_str);
	for (auto it = fs->begin(); it != fs->end(); ++it) {
	 	Fact* fact = (*it);
      	cout << "f_id: " << fact->f_id << endl;
    }
	cout << fs << endl;
}

void bench_build(){
	FactSet* fs;
	char* json_str;
	time_it_n("individual", fs=test_individual_build(1000), 1000);
	time_it_n("\ttraverse:", loop_fact_set(fs), 1000);
	time_it_n("unbuffered", fs=test_unbuffered_build(1000), 1000);
	time_it_n("\ttraverse:", loop_fact_set(fs), 1000);
	time_it_n("buffered", fs=test_buffered_build(1000, 3), 1000);
	time_it_n("\ttraverse:", loop_fact_set(fs), 1000);
	json_str = FactSet_to_json(fs);
	time_it_n("from_json", fs=FactSet_from_json(json_str), 20);
	time_it_n("\ttraverse:", loop_fact_set(fs), 1000);

	time_it_n("_update_init", (new Flattener(fs))->_update_init();, 500);
}

uint64_t do_stuff(const string_view& x) {
    // return fnv1a((const uint8_t*) x.data(), x.size());
    return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
}

void test_json(){
	FactSet* fs;
	fs = test_buffered_build(10, 3);	
	cout << fs << endl;
	auto json_str = FactSet_to_json(fs);	
	cout << "-START2-" << endl;
	cout << json_str << endl;
	fs = FactSet_from_json(json_str);
	cout << "-START3-" << endl;
	fs = FactSet_from_json(json_str);
	cout << "END" << endl;
}


int main() {
	// FactSet* fs;
	// cout << "HELLO" << endl;
	// cout << "CRE_Context: " << cre_Fact->name << endl;
	// cout << "CRE_Context: " << default_context->name << endl;

	bench_build();
	// test_errors();
	// test_flags();
	// test_iterate_fact();
	// test_json();
	cout << "AND ITS DONE" << endl;



	// // // ----- UNCOMMENT BENCHMARKS ---------
	// fs = test_individual_build(10, 3);
	// cout << fs << endl;
	// fs = test_unbuffered_build(10, 3);
	// cout << fs << endl;
	// fs = test_buffered_build(10, 3);	
	// cout << fs << endl;

	// cout << "-START-" << endl;
	
	

	

	
}
