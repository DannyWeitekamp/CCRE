// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <cstdint>
#include <cstring>

#include "../include/helpers.h"
#include "../include/types.h"
#include "../include/hash.h"
#include "../include/context.h"
#include "../include/item.h"
#include "../include/fact.h"
#include "../include/factset.h"
#include "../include/intern.h"

// #include "../include/unicode.h"
#include "../include/cre_obj.h"
#include "../include/wref.h"
#include "../include/pool_allocator.h"
#include "test_macros.h"

// #include <chrono>
// using namespace std;
// using namespace std::chrono;

using std::cout;
using std::endl;
using namespace cre;

void test_float_str(){
	IS_TRUE(flt_to_str(1.0) == "1.0")
	IS_TRUE(flt_to_str(1.0000001) == "1.0000001")
	IS_TRUE(flt_to_str(1.0/3) == "0.3333333333333333")
	IS_TRUE(flt_to_str(1e26) == "1e+26")
	IS_TRUE(flt_to_str(-1e+26) == "-1e+26")
	IS_TRUE(flt_to_str(1.0786500e+26) == "1.07865e+26")

	IS_TRUE(int_to_str(7) == "7")
	IS_TRUE(int_to_str(-7) == "-7")
	IS_TRUE(int_to_str(9223372036854775807) == "9223372036854775807")
	IS_TRUE(int_to_str(-9223372036854775807) == "-9223372036854775807")
}


void test_item_size(){
	IS_TRUE(sizeof(Item) == 16);
}

void test_item_truthiness(){
	IS_TRUE(bool(Item()) == false);
	IS_TRUE(bool(Item(None)) == false);
	IS_TRUE(bool(Item(false)) == false);
	IS_TRUE(bool(Item(0)) == false);
	IS_TRUE(bool(Item(0.0)) == false);
	IS_TRUE(bool(Item("")) == false);
	IS_TRUE(bool(Item(true)) == true);
	IS_TRUE(bool(Item(1)) == true);
	IS_TRUE(bool(Item(1.0)) == true);
	IS_TRUE(bool(Item("a")) == true);
}

void test_item_eq(){

	IS_TRUE(Item() == Item());
	IS_TRUE(Item() != false);

	IS_TRUE(Item(None) == Item(None));
	IS_TRUE(Item(None) != false);

	std::string s1 = "ab";
	std::string s2 = "ac";
	IS_TRUE(Item("ab") == Item("ab"));
	IS_TRUE(Item("ab") == "ab");
	IS_TRUE(Item("ab") == intern("ab"));
	IS_TRUE(Item("ab") == std::string_view(s1));
	IS_TRUE(Item("ab") != Item("ac"));
	IS_TRUE(Item("ab") != "ac");
	IS_TRUE(Item("ab") != intern("ac"));
	IS_TRUE(Item("ab") != std::string_view(s2));

	IS_TRUE(Item(true) == Item(true));
	IS_TRUE(Item(true) == true);
	IS_TRUE(Item(true) == Item(1));
	IS_TRUE(Item(true) == 1);
	IS_TRUE(Item(true) == Item(1.0));
	IS_TRUE(Item(true) == 1.0);
	IS_TRUE(Item(1) == Item(true));
	IS_TRUE(Item(1) == true);
	IS_TRUE(Item(1) == Item(1));
	IS_TRUE(Item(1) == 1);
	IS_TRUE(Item(1) == Item(1.0));
	IS_TRUE(Item(1) == 1.0);
	IS_TRUE(Item(1.0) == Item(true));
	IS_TRUE(Item(1.0) == true);
	IS_TRUE(Item(1.0) == Item(1));
	IS_TRUE(Item(1.0) == 1);
	IS_TRUE(Item(1.0) == Item(1.0));
	IS_TRUE(Item(1.0) == 1.0);

	IS_TRUE(Item("1") != Item(1));
	IS_TRUE(Item("1.0") != Item(1.0));

}


void test_errors(){
	FactType* CatType = define_fact("Cat", 
	    {{"id", cre_str},
	     {"color", cre_str},
	     {"legs", cre_int},
	     {"frisky", cre_bool}
	 	}
	);
	// Okay
	make_fact(CatType, "snowball", "white", 3, false);

	// Errors
	EXPECT_THROW(make_fact(CatType, 0, "white", 3, false));
	EXPECT_THROW(make_fact(CatType, "snowball", "white", 3.0, false));
	
}

void test_flags(){
	FactType* CatType = define_fact("Cat", 
	    {{"id", cre_str, {{"unique_id", Item(true)}} },
	     {"color", cre_str},
	     {"legs", cre_int, {{"verbosity", Item(2)}}},
	     {"frisky", cre_bool}
	 	}
	);
	// // Okay
	ref<Fact> snowball = make_fact(CatType, "snowball", "white", 3, false);

	IS_TRUE(snowball->type->members[0].get_flag(BIFLG_UNIQUE_ID) == true);
	IS_TRUE(snowball->type->members[1].get_flag(BIFLG_UNIQUE_ID) == false);
	IS_TRUE(snowball->type->members[2].get_flag(BIFLG_UNIQUE_ID) == false);
	IS_TRUE(snowball->type->members[3].get_flag(BIFLG_UNIQUE_ID) == false);

	IS_TRUE(snowball->type->members[0].get_flag(BIFLG_VERBOSITY) == 1);
	IS_TRUE(snowball->type->members[1].get_flag(BIFLG_VERBOSITY) == 1);
	IS_TRUE(snowball->type->members[2].get_flag(BIFLG_VERBOSITY) == 2);
	IS_TRUE(snowball->type->members[3].get_flag(BIFLG_VERBOSITY) == 1);

	cout << snowball->to_string(0) << endl;
	cout << snowball->to_string(1) << endl;
	cout << snowball->to_string(2) << endl;
	cout << snowball->to_string(3) << endl;
}

void test_iterate_fact(){
	ref<Fact> fact = make_fact(NULL, "A", 0, false, "Q");

	cout << "LENGTH: " << fact->length << endl;
	// for(auto& item : &fact){
	for (auto it = fact->begin(); it != fact->end(); ++it) {
		Item item = *it;
		cout << item << endl;
	}
	for(auto _item : *fact){
		cout << _item << endl;
	}
	for(auto _item : fact){
		cout << _item << endl;
	}

	cout << endl;
	auto fs = FactView(fact,0,3);
	cout << "FactView(fact,0,3) L=" << fs.size() << endl;	
	int i = 0;
	for(auto _item : fs){
		cout << _item << " " << fs[i] << endl;
		i++;
	}

	cout << endl;
	auto fs2 = FactView(fact, 1, -2);
	cout << "FactView(fact, 1, -2) L=" << fs2.size() << endl;
	i = 0;
	for(auto _item : fs2){
		cout << _item << " " << fs2[i] << endl;
		i++;
	}

	cout << endl;
	auto fs3 = FactView(fact,-3,3);
	cout << "FactView(fact,-3,3) L=" << fs3.size() << endl;
	i = 0;
	for(auto _item : fs3){
		cout << _item << " " << fs3[i] << endl;
		i++;
	}

	cout << endl;
	auto fs4 = FactView(fact,-4,-3);
	cout << "FactView(fact,-4,-3) L=" << fs4.size() << endl;
	i = 0;
	for(auto _item : fs4){
		cout << _item << " " << fs4[i] << endl;
		i++;
	}

	EXPECT_THROW(( FactView(fact,3, 0) ));
	EXPECT_THROW(( FactView(fact, 2,-3) ));
}

void test_hash(){
	// auto fact1 = empty_fact(nullptr, 1);
	auto fact2 = empty_fact(nullptr, 2);
	auto fact3 = empty_fact(nullptr, 3);
	uint64_t zhash;

	// All empty_facts should have a valid length-dependant hash
	IS_TRUE(fact2->hash != 0);
	zhash = CREHash{}(fact3);
	IS_TRUE(zhash != 0);
	IS_TRUE(zhash != fact2->hash);

	// Set() should immediately modify the fact's hash field without 
	//  the need to perform a rehash. 
	fact3->set(0, "BOB");
	fact3->set(1, "BRAD");
	fact3->set(2, 3.0);
	IS_TRUE(fact3->hash != zhash);

	// make_fact(), and by extension new_fact() should produce facts with 
	//  the same hash as ones constructed by calling set().
	auto fact3p = make_fact(nullptr, "BOB", "BRAD", 3.0);
	IS_TRUE(fact3p->hash == fact3->hash);

	// Facts with the same Members, but in different orders should have
	//  different hashes. 
	auto fact3_unord = make_fact(nullptr, "BRAD", "BOB", 3.0);
	IS_TRUE(fact3p->hash != fact3_unord->hash);
	fact3->set(0, "BRAD");
	fact3->set(1, "BOB");
	fact3->set(2, 3.0);
	IS_TRUE(fact3->hash == fact3_unord->hash);

	// Setting all members to nullptr should produce the same hash
	//  as an empty_fact() of the same size.
	fact3->set(0, nullptr);
	fact3->set(1, nullptr);
	fact3->set(2, nullptr);
	IS_TRUE(fact3->hash == zhash);

	// Size 1 facts shouldn't produce hash conflicts
	// (for instance if multiplying the member hash by ind=0)
	IS_TRUE((
		make_fact(nullptr, "BOB")->hash != 
		make_fact(nullptr, "BRAD")->hash
	))

	// A full span FactView should have the same hash as its parent fact. 
	IS_TRUE((
		CREHash{}(FactView(fact3p, 0,3)) ==
		CREHash{}(fact3p)
	))

	// A partial span FactView should NOT have the same hash as its parent fact. 
	IS_TRUE((
		CREHash{}(FactView(fact3p, 1,3)) !=
		CREHash{}(fact3p)
	))

	// A fact should never have a hash of 0
	// ??
}

auto nested_objects(){
	FactType* CatType = define_fact("Cat", 
	    {{"id", cre_str, {{"unique_id", Item(true)}} },
	     {"color", cre_str},
	     {"legs", cre_int, {{"verbosity", Item(2)}}},
	     {"frisky", cre_bool}
	 	}
	);
	FactType* CatOwner = define_fact("CatOwner", 
	    {{"id", cre_str, {{"unique_id", Item(true)}} },
	     {"cat", CatType},
	     {"fudge", cre_Fact},
	     {"buddy", new DefferedType("CatOwner")}
	 	}
	);

	ref<Fact> fudge = make_tuple("This", "is", "fudge");
	cout << "FUDGE TYPE: " << uint64_t(fudge->type) << endl;

	ref<Fact> snowball = make_fact(CatType, "snowball", "white", 77, false);

	ref<Fact> Jeff = make_fact(CatOwner, "Jeff", snowball, fudge);
	ref<Fact> double_fudge = make_tuple("double", fudge);
	ref<Fact> Bobby = make_fact(CatOwner, "Bobby", snowball, double_fudge, Jeff);	
	return std::make_tuple(fudge, snowball, Jeff, double_fudge, Bobby);
}

auto test_to_string(){
	auto [fudge, snowball, Jeff, double_fudge, Bobby] = nested_objects();

	cout << "START PRINT 0" << endl;
	cout << fudge->to_string() << endl;
	cout << "START PRINT 1" << endl;
	cout << snowball->to_string() << endl;
	cout << "START PRINT 2" << endl;
	cout << Jeff->to_string() << endl;
	cout << "START PRINT 3" << endl;
	cout << double_fudge->to_string() << endl;
	cout << "START PRINT 4" << endl;
	cout << Bobby->to_string() << endl;

}

void test_copy(){
	
	auto [fudge, snowball, Jeff, double_fudge, Bobby] = nested_objects();

	// cout << "FUDGE: " << fudge->get_refcount() << endl;
	// cout << "SNOWBALL: " << snowball->get_refcount() << endl;

	ref<Fact> snowball_copy = snowball->copy();
	ref<Fact> Jeff_copy = Jeff->copy(COPY_SHALLOW);
	ref<Fact> Jeff_deep_copy = Jeff->copy(COPY_DEEP);
	ref<Fact> Jeff_deep_ref_copy = Jeff->copy(COPY_DEEP_REFS);

	// Copies should have the same string as originals
	IS_TRUE(snowball->to_string() == snowball_copy->to_string());
	
	// SHALLOW copies should just copy pointers 
	ref<Fact> Jeff_copy_cat = Jeff_copy->get("cat").as<Fact*>(); 
	IS_TRUE(Jeff->to_string() == Jeff_copy->to_string());
	IS_TRUE(uint64_t(Jeff_copy_cat.get()) == uint64_t(snowball.get()));

	// DEEP copies should copy whole objects if they are not references
	IS_TRUE(Jeff->to_string() == Jeff_deep_copy->to_string());
	IS_TRUE(uint64_t(Jeff_deep_copy.get()) != uint64_t(snowball.get()));

	// DEEP_REFS copies should copy whole objects even if they are references
	IS_TRUE(Jeff->to_string() == Jeff_deep_ref_copy->to_string());
	IS_TRUE(uint64_t(Jeff_deep_ref_copy.get()) != uint64_t(snowball.get()));


	cout << double_fudge << endl;
	cout << Bobby << endl;

}

void test_slice(){
	
}


void test_pool_alloc(){
	// Assumes sizeof(Block) == 64 (i.e. the header of a block)
	uint64_t block_size = 64 + 8*(2*sizeof(void*)+sizeof(ControlBlock));

	auto pool = PoolAllocator<ControlBlock>(block_size);
	auto stats = pool.get_stats();

	std::vector<ControlBlock*> blocks = {};
	for(int i=0; i < 35; i++){
		ControlBlock* block = pool.alloc();
		blocks.push_back(block);
	}

	// All but 5 chunk should be used
	stats = pool.get_stats();
	// cout << stats << endl;
	IS_TRUE(stats.n_blocks == 5);
	IS_TRUE(stats.allocated_chunks == 40);
	IS_TRUE(stats.used_chunks == 35);
	IS_TRUE(stats.free_chunks == 5);

	for (auto it = blocks.begin(); it != blocks.end(); ++it) {
		ControlBlock* block = *it;
		pool.dealloc(block);
	}

	// There should be just one active block w/ 8 free chunks
	stats = pool.get_stats();
	// cout << stats << endl;
	IS_TRUE(stats.n_blocks == 1);
	IS_TRUE(stats.allocated_chunks == 8);
	IS_TRUE(stats.used_chunks == 0);
	IS_TRUE(stats.free_chunks == 8);
	
	cout << "-----------------------" << endl;

	std::vector<ControlBlock*> odd_blocks = {};
	for(int i=0; i < 35; i++){
		ControlBlock* block = pool.alloc();
		if(i % 2){
			odd_blocks.push_back(block);
		}
	}

	// cout << "odd_blocks: " << odd_blocks.size() << endl;
	for (auto it = odd_blocks.begin(); it != odd_blocks.end(); ++it) {
		ControlBlock* block = *it;
		pool.dealloc(block);
	}
	for(int i=0; i < 10; i++){
		ControlBlock* block = pool.alloc();
	}
	
	// Should be several active blocks
	stats = pool.get_stats();
	IS_TRUE(stats.n_blocks == 5);
	IS_TRUE(stats.allocated_chunks == 40);
	IS_TRUE(stats.used_chunks == 28);
	IS_TRUE(stats.free_chunks == 12);

	// ---------------------------------
	// : Test alloc_batch

	cout << "-----------------------" << endl;

	// cout << pool.get_stats() << endl;

	auto batch = pool.alloc_batch(10);
	for (auto it = batch.begin(); it != batch.end(); ++it) {
		cout << uint64_t(&*it) << endl;;
	}
	// cout << pool.get_stats() << endl;

	stats = pool.get_stats();
	IS_TRUE(stats.n_blocks == 7);
	IS_TRUE(stats.allocated_chunks == 56);
	IS_TRUE(stats.used_chunks == 38);
	IS_TRUE(stats.free_chunks == 18);
}

void bench_pool_alloc(){
	
	// std::vector<ControlBlock*> pool_allocs = {};
	
	const size_t N = 20000;
	std::string N_str = std::to_string(N);


	ControlBlock* bb = nullptr;
	time_it_n(N_str + " malloc:     ", 
		for(int i=0; i < N; i++){
			ControlBlock* block = (ControlBlock*) malloc(sizeof(ControlBlock));
			block->dtor = nullptr;
			bb = block;
			// reg_mallocs.push_back(block);
		}
	,200);	
	
	time_it_n(N_str + " Pool alloc: ", 
		auto pool = PoolAllocator<ControlBlock>();	
		for(int i=0; i < N; i++){
			ControlBlock* block = pool.alloc();
			block->dtor = nullptr;
			// bb = block;
			// pool_allocs.push_back(block);
		}	
	,200);

	time_it_n("10x" + N_str + " Batch alloc: ", 
		auto pool = PoolAllocator<ControlBlock>();
		for(int i=0; i < N/1000; i++){
			auto batch = pool.alloc_batch(1000);
			for (auto it = batch.begin(); it != batch.end(); ++it) {
				ControlBlock* block = &*it;
				block->dtor = nullptr;
				// bb = block;
				// cout << uint64_t(&*it) << endl;;
			}
		}	
	,200);

	time_it_n(N_str + " Pool alloc_forward: ", 
		auto pool = PoolAllocator<ControlBlock>();	
		for(int i=0; i < N; i++){
			ControlBlock* block = pool.alloc_forward();
			block->dtor = nullptr;
			// bb = block;
			// pool_allocs.push_back(block);
		}	
	,200);

	

	

	// This prevents the compiler from not running malloc code
	cout << uint64_t(bb) << endl;

	// std::vector<ControlBlock*> reg_mallocs = {};
	
	

	

	// Make a few more so they can be freed
	auto pool = PoolAllocator<ControlBlock>();
	std::vector<ControlBlock*> pool_allocs = {};
	for(int i=0; i < N; i++){
		ControlBlock* block = pool.alloc();
		pool_allocs.push_back(block);
	}	

	std::vector<ControlBlock*> reg_mallocs = {};
	for(int i=0; i < N; i++){
		ControlBlock* block = (ControlBlock*) malloc(sizeof(ControlBlock));
		reg_mallocs.push_back(block);
	}


	time_it(N_str + " Pool dealloc:", 
		for (auto it = pool_allocs.begin(); it != pool_allocs.end(); ++it) {
			ControlBlock* block = *it;
			pool.dealloc(block);
		}	
	);

	time_it(N_str + " free:        ", 
		for (auto it = reg_mallocs.begin(); it != reg_mallocs.end(); ++it) {
			ControlBlock* block = *it;
			free(block);
		}	
	);


	// auto batch = pool.alloc_batch(10);
	// for (auto it = batch.begin(); it != batch.end(); ++it) {
	// 	cout << uint64_t(&*it) << endl;;
	// }
}


void test_weakref(){
	auto [fudge, snowball, Jeff, double_fudge, Bobby] = nested_objects();

	// cout << Jeff->get("cat") << endl;
	IS_TRUE(Jeff->get("cat") == snowball);

	IS_TRUE(snowball->get_wrefcount() == 2);
	IS_TRUE(snowball->get_refcount()  == 1);


	// cout << "W_REF: " << snowball->get_wrefcount() << 
	//  		", S_REF: " << snowball->get_refcount() << endl;

	wref<Fact> snowball_wref = snowball;
	wref<Fact> Bobby_wref = Bobby;

	IS_TRUE(snowball->get_wrefcount() == 3);
	IS_TRUE(snowball->get_refcount()  == 1);



	// cout << "W_REF: " << snowball_wref.get_wrefcount() << 
	//  		", S_REF: " << snowball_wref.get_refcount() << endl;
	
	// cout << "BOBBY: " << Bobby->get_refcount() <<  endl;

	snowball = NULL;
	Bobby = NULL;

	cout << Jeff->to_string() << endl;
	IS_TRUE(Jeff->to_string().find("cat=expired[@snowball]") != std::string::npos);



	IS_TRUE(snowball_wref.is_expired());
	IS_TRUE(Bobby_wref.is_expired());

	// cout << "BOBBY: " << Bobby_wref.get_refcount() <<  endl;	

	// Should throw an error since snowball has expired
	EXPECT_THROW(( snowball_wref->get_wrefcount() ));

	IS_TRUE(snowball_wref.get_wrefcount() == 2);

	// cout << "!W_REF: " << snowball_wref.get_wrefcount() << 
	//  		", S_REF: " << snowball_wref.get_refcount() << endl;

	Jeff = NULL;

	IS_TRUE(snowball_wref.get_wrefcount() == 1);

	// IS_TRUE(snowball_wref.is_expired());

	// if(snowball_wref == nullptr){
	// 	cout << "nullptr" << endl;
	// }

	// cout << "W_REF: " << snowball_wref.get_wrefcount() << 
	//  		", S_REF: " << snowball_wref.get_refcount() << endl;

	snowball_wref = nullptr;

	IS_TRUE(snowball_wref.get_wrefcount() == 0);

	// cout << snowball_wref.get_wrefcount() << endl;

	// auto Jeffs_cat = Jeff->get("cat").as<Fact*>();
	// cout << Jeffs_cat << "," << Jeffs_cat->get_refcount() << endl;

	// cout << "END"<< endl;
}

void bench_deref(){
	FactType* Person = define_fact("Person", 
	    {{"id", cre_str, {{"unique_id", Item(true)}} },
	     {"buddy", new DefferedType("Person")}
	 	}
	);

	const int M = 200;	
	const int N = 50000;

	vector<ref<Fact>> people = {};
	Fact* buddy = nullptr;
	for(int i=0; i < N; i++){
		// cout << "i: " << i << endl;
		ref<Fact> person = make_fact(Person, "person" + std::to_string(i), buddy);
		people.push_back(person);

		// cout << "REFCOUNT: " << person->get_refcount() << endl;

		buddy = person;
	}
	
	
	// time_it("Deref slow: ", 
	// 	for(int j=0; j < M; j++){
	// 		Fact* person = people[N-1];	
	// 		for(int i=0; i < N-1; i++){
	// 			// cout << person->get("id") << endl;
	// 			person = person->get(1).as<Fact*>_slow();
	// 		}
	// 	}
	// );

	time_it("Deref fast: ", 
		for(int j=0; j < M; j++){
			Fact* person = people[N-1];	
			for(int i=0; i < N-1; i++){
				person = person->get(1).as<Fact*>();
			}
		}
	);

	


	


	

}



int main() {
	test_float_str();
	// test_item_size();
	// test_item_eq();
	// test_item_truthiness();
    // test_errors();
    // test_flags();
    // test_iterate_fact();
    // test_hash();
    // test_to_string();
    // test_copy();
    // test_pool_alloc();
    // bench_pool_alloc();
    // test_weakref();
	// bench_deref();
    return 0;
}
