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


using std::cout;
using std::endl;


int loop_fact_set(ref<FactSet> fs){
	volatile int x = 0;
	for (auto it = fs->begin(); it != fs->end(); ++it) {
	 	ref<Fact> fact = (*it);
	 	x = x + fact->f_id;
      // cout << "f_id: " << (&(*it))->f_id << endl;
      // state->print_layout();
      // retract(state, (&(*it))->f_id);
    }
    return x;
}

ref<FactSet> test_individual_build(size_t N){
	ref<FactSet> fs = new FactSet();
	// Item str_item = Item("A");
	std::vector<Item> items = {Item(0), Item("A"), Item(false)};
	for(int i=0; i < N; i++){
		// std::vector<Item> items = {Item(i), str_item, Item(bool(i%2))};
		ref<Fact> fact = new_fact(NULL, items.data(), items.size());
		declare(fs, fact);
	}
	return fs;
}

ref<FactSet> test_unbuffered_build(size_t N){
	FactSetBuilder fs_builder = FactSetBuilder(N, 0);
	// cout << fs_builder->fact_set << endl;
	// Item str_item = Item("A");
	std::vector<Item> items = {Item(0), Item("A"), Item(false)};
	for(int i=0; i < N; i++){
		fs_builder.add_fact(nullptr, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder.fact_set;
}


ref<FactSet> test_buffered_build(size_t N){
	FactSetBuilder fs_builder = FactSetBuilder(N, N*(SIZEOF_FACT(3)));
	// cout << fs_builder->fact_set << endl;
	// Item str_item = Item("A");
	std::vector<Item> items = {Item(0), Item("A"), Item(false)};
	for(int i=0; i < N; i++){
		fs_builder.add_fact(nullptr, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder.fact_set;
}

void test_build_from_json(){
	char* json_str;
	ref<FactSet> fs;
	define_fact("Cat", 
	    {{"id", cre_str},
	     {"color", cre_str},
	     {"legs", cre_int},
	     {"frisky", cre_bool}
	 	}
	);
	cout << "HELLO2" << endl;

	fs = FactSet::from_json_file((char *) "../data/small_test_state.json");
	cout << "AFTER FS" << endl;
	cout << fs << endl;

	json_str = FactSet::to_json(fs);

	

	cout << "Written JSON str" << endl;
	cout << json_str << endl;
	
	time_it_n("from_json_file", fs=FactSet::from_json_file("../data/big_untyped_state.json"), 50);
	time_it_n("to_json", FactSet::to_json(fs), 50);

	fs = new FactSet();
	ref<Fact> a = fs->add_fact(nullptr, {Item(0), Item("A"), Item(false)});
	ref<Fact> b = fs->add_fact(nullptr, {Item(1), Item("B"), Item(true)});
	ref<Fact> c = fs->add_fact(nullptr, {Item(2), Item("C"), Item(false)});
	fs->add_fact(nullptr, {Item(a), Item(b), Item(c)});

	cout << fs << endl;
	json_str = FactSet::to_json(fs);
	cout << json_str << endl;

	fs=FactSet::from_json(json_str);
	for (auto it = fs->begin(); it != fs->end(); ++it) {
	 	ref<Fact> fact = (*it);
      	cout << "f_id: " << fact->f_id << endl;
    }
	cout << fs << endl;
	delete[] json_str;
}

void bench_build(size_t N=1000, size_t reps=500){
	ref<FactSet> fs;
	ref<Flattener> flattener;
	char* json_str;
	time_it_n("individual", fs=test_individual_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	time_it_n("unbuffered", fs=test_unbuffered_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	time_it_n("buffered", fs=test_buffered_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	json_str = FactSet::to_json(fs);
	time_it_n("from_json", fs=FactSet::from_json(json_str), 20);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);

	time_it_n("_update_init", (flattener=new Flattener(fs))->_update_init();, reps);
	delete[] json_str;
}

uint64_t do_stuff(const std::string_view& x) {
    // return fnv1a((const uint8_t*) x.data(), x.size());
    return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
}

void test_json(){
	ref<FactSet> fs;
	fs = test_buffered_build(10);	
	cout << fs << endl;
	auto json_str = FactSet::to_json(fs);	
	cout << "-START2-" << endl;
	cout << json_str << endl;
	fs = FactSet::from_json(json_str);
	cout << endl << fs << endl;
	cout << "-START3-" << endl;
	fs = FactSet::from_json(json_str);
	cout << endl << fs << endl;
	cout << "END" << endl;

	delete[] json_str;
}


int main() {
	
	// cout << "HELLO" << endl;
	// cout << "CRE_Context: " << cre_Fact->name << endl;
	// cout << "CRE_Context: " << default_context->name << endl;
	test_json();
	bench_build(1000,500);
	
	cout << "AND ITS DONE" << endl;



	// // // ----- UNCOMMENT BENCHMARKS ---------
	// ref<FactSet> fs;
	// fs = test_individual_build(10);
	
	// // cout << fs << endl;
	// fs = test_unbuffered_build(10);
	// // cout << fs << endl;
	// fs = test_buffered_build(10);	
	// cout << fs << endl;

	// cout << "-START-" << endl;
	
	

	

	
}
