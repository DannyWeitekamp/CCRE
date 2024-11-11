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


int main(){
    test_errors();
    test_flags();
    test_iterate_fact();
    return 0;
}
