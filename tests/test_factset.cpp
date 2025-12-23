#include <inttypes.h>              // for uint64_t, uint8_t
#include <cstring>                 // for size_t
#include <iostream>                // for basic_ostream, operator<<, endl, cout
#include <string>                  // for char_traits, operator<<, string
#include <string_view>             // for string_view
#include <vector>                  // for vector
#include "../include/context.h"    // for global_cb_pool
#include "../include/cre_obj.h"    // for ControlBlock
#include "../include/fact.h"       // for Fact, _SIZEOF_FACT, make_fact, SIZ...
#include "../include/factset.h"    // for FactSet, operator<<, FactSetBuilder
#include "../include/flattener.h"  // for Flattener
#include "../include/hash.h"       // for MurmurHash64A
#include "../include/item.h"       // for Item
#include "../include/types.h"      // for MemberSpec, cre_str, cre_bool, cre...
#include "pool_allocator.h"        // for operator<<, PoolAllocator
#include "ref.h"                   // for ref
#include "test_macros.h"           // for time_it_n, IS_TRUE, EXPECT_THROW
#include "wref.h"                  // for wref


using std::cout;
using std::endl;
using namespace cre;


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
	std::vector<Item> items = {0, "A", false};
	for(int i=0; i < N; i++){
		// std::vector<Item> items = {Item(i), str_item, Item(bool(i%2))};
		ref<Fact> fact = new_fact(nullptr, items.data(), items.size());
		declare(fs, fact);
	}
	return fs;
}

ref<FactSet> test_unbuffered_build(size_t N){
	FactSetBuilder fs_builder = FactSetBuilder(N, 0);
	// cout << fs_builder->fact_set << endl;
	// Item str_item = Item("A");
	std::vector<Item> items = {0, "A", false};
	for(int i=0; i < N; i++){
		fs_builder.declare_new(nullptr, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder.fact_set;
}


ref<FactSet> test_buffered_build(size_t N){
	FactSetBuilder fs_builder = FactSetBuilder(N, N*(SIZEOF_FACT(3)));
	// cout << fs_builder->fact_set << endl;
	// Item str_item = Item("A");
	std::vector<Item> items = {0, "A", false};
	for(int i=0; i < N; i++){
		fs_builder.declare_new(nullptr, items.data(), items.size());
		// cout << fact << endl;
	}
	return fs_builder.fact_set;
}


void test_get(){
	ref<FactSet> fs = test_buffered_build(10);

	EXPECT_THROW(ref<Fact> fact = fs->get(100));
	fs->retract(7);
	EXPECT_THROW(ref<Fact> fact = fs->get(7));

	cout << "Fact 0: " << fs->get(0) << endl;
}

void test_memleak(){
	ref<FactSet> fs = test_buffered_build(1);
	wref<FactSet> fs_wref = wref<FactSet>(fs);
	ref<Fact> fact = fs->get(0);
	wref<Fact> fact_wref = wref<Fact>(fact);

	IS_TRUE(fs_wref.get_refcount() == 1);
	IS_TRUE(fact.get_refcount() == 2);
	// cout << "FS RC: " <<  << endl;
	// cout << "F RC: " << fact.get_refcount() << endl;

	fs = nullptr;

	IS_TRUE(fs_wref.get_refcount() == 0);
	IS_TRUE(fact.get_refcount() == 1);

	// cout << "FS RC: " << fs_wref.get_refcount() << endl;
	// cout << "F RC: " << fact.get_refcount() << endl;

	fact = nullptr;

	IS_TRUE(fact.get_refcount() == 0);
	IS_TRUE(fact_wref.get_wrefcount() == 1);

	// cout << "FS RC: " << fs_wref.get_refcount() << endl;
	// cout << "F RC: " << fact_wref.get_refcount() << endl;
	// cout << "F WRC: " << fact_wref.get_wrefcount() << endl;
	fact_wref = nullptr;	
}

void test_build_from_json(){
	std::string json_str;
	ref<FactSet> fs;
	define_fact("Cat", 
	    {{"id", cre_str},
	     {"color", cre_str},
	     {"legs", cre_int},
	     {"frisky", cre_bool}
	 	}
	);
	cout << "HELLO2" << endl;

	fs = FactSet::from_json_file("../data/small_test_state.json");
	cout << "AFTER FS" << endl;
	cout << fs << endl;

	json_str = fs->to_json();
	cout << "Written JSON str" << endl;
	cout << json_str << endl;
	// delete[] json_str;
	#if NDEBUG
		cout << "NOT DEBUG!!" << endl;
		time_it_n("from_json_file", fs=FactSet::from_json_file("../data/big_untyped_state.json"), 50);
		time_it_n("to_json", json_str = fs->to_json(), 50);
	#endif

	fs = new FactSet();
	ref<Fact> a = make_fact(nullptr, 0, "A", false);
	ref<Fact> b = make_fact(nullptr, 1, "B", true);
	ref<Fact> c = make_fact(nullptr, 2, "C", false);
	fs->declare(a); fs->declare(b); fs->declare(c);
	fs->declare(make_fact(nullptr, a, b, c));

	cout << fs << endl;
	json_str = fs->to_json();
	cout << json_str << endl;

	fs=FactSet::from_json(json_str);
	for (auto it = fs->begin(); it != fs->end(); ++it) {
	 	ref<Fact> fact = (*it);
      	cout << "f_id: " << fact->f_id << endl;
    }
	cout << fs << endl;
	// delete[] json_str;
}

void bench_build(size_t N=1000, size_t reps=500){
	ref<FactSet> fs;
	ref<Flattener> flattener;
	std::string json_str;
	time_it_n("individual", fs=test_individual_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	time_it_n("unbuffered", fs=test_unbuffered_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	time_it_n("buffered", fs=test_buffered_build(N), reps);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);
	json_str = fs->to_json();
	time_it_n("from_json", fs=FactSet::from_json(json_str), 20);
	time_it_n("\ttraverse:", loop_fact_set(fs), reps);

	time_it_n("_update_init", (flattener=new Flattener(fs))->_update_init();, reps);
	// delete[] json_str;
}

uint64_t do_stuff(std::string_view x) {
    // return fnv1a((const uint8_t*) x.data(), x.size());
    return MurmurHash64A((const uint8_t*) x.data(), x.size(), 0xFF);
}

void test_json(){
	ref<FactSet> fs;
	fs = test_buffered_build(10);	
	cout << fs << endl;
	auto json_str = fs->to_json();	
	cout << "-START2-" << endl;
	cout << json_str << endl;
	fs = FactSet::from_json(json_str);

	
	cout << endl << fs << endl;
	cout << "-START3-" << endl;
	fs = FactSet::from_json(json_str);
	cout << endl << fs << endl;
	cout << "END" << endl;
	// delete[] json_str;
}


int main() {
	cout << "ALIGN=" << FACT_ALIGN << ", NEED PAD:" << FACT_NEED_ALIGN_PAD << endl;
	cout << _SIZEOF_FACT(0) << "," << SIZEOF_FACT(0) << "," << ALIGN_PADDING(_SIZEOF_FACT(0)) << endl;
	cout << _SIZEOF_FACT(1) << "," << SIZEOF_FACT(1) << "," << ALIGN_PADDING(_SIZEOF_FACT(1)) << endl;
	cout << _SIZEOF_FACT(2) << "," << SIZEOF_FACT(2) << "," << ALIGN_PADDING(_SIZEOF_FACT(2)) << endl;
	cout << _SIZEOF_FACT(3) << "," << SIZEOF_FACT(3) << "," << ALIGN_PADDING(_SIZEOF_FACT(3)) << endl;
	cout << _SIZEOF_FACT(4) << "," << SIZEOF_FACT(4) << "," << ALIGN_PADDING(_SIZEOF_FACT(4)) << endl;
	cout << _SIZEOF_FACT(5) << "," << SIZEOF_FACT(5) << "," << ALIGN_PADDING(_SIZEOF_FACT(5)) << endl;
	cout << _SIZEOF_FACT(6) << "," << SIZEOF_FACT(6) << "," << ALIGN_PADDING(_SIZEOF_FACT(6)) << endl;
	
	// cout << "HELLO" << endl;
	// cout << "CRE_Context: " << cre_Fact->name << endl;
	// cout << "CRE_Context: " << default_context->name << endl;

	// Note: Doing this test before the benchmark affects the outcome
	//   since mallocing on a fresh process is simialr to the buffered allocs
	
	
	
	cout << global_cb_pool.get_stats() << endl;
		
	#if NDEBUG
		cout << "RUNNING RELEASE" << endl;
		bench_build(1000,500);
	#else
		cout << "RUNNING DEBUG" << endl;	
		bench_build(1000,5);
		// bench_build(10,10);
	#endif
	test_memleak();

	cout << global_cb_pool.get_stats() << endl;

	
	test_build_from_json();
	test_json(); 
	test_get();
	test_memleak();
	




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
