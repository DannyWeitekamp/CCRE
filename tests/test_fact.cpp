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

// #include <chrono>
// using namespace std;
// using namespace std::chrono;

using std::cout;
using std::endl;


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

void test_copy(){

}

void test_slice(){
	
}





int main(){
    // test_errors();
    // test_flags();
    // test_iterate_fact();

    test_hash();
    return 0;
}
