// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <cstdint>
#include <cstring>

#include "test_macros.h"

#include <chrono>
using namespace std;
using namespace std::chrono;

#include "../include/var.h"
#include "../include/fact.h"

void test_var(){
	FactType* DudeType = define_fact("Dude", 
	    {{"name", cre_str, {{"unique_id", Item(true)}}},
	     {"buddy", new DefferedType("Dude")},
	     {"dad", new DefferedType("Dude")},
	     {"mom", new DefferedType("Dude")}
	 	}
	);

	Fact* olpops = make_fact(DudeType, "Ol'Pops");
	Fact* pops = make_fact(DudeType, "Pops", nullptr, olpops, nullptr);
	Fact* ma = make_fact(DudeType, "Ma", pops);
	Fact* ricky = make_fact(DudeType, "Ricky");
	Fact* thedude = make_fact(DudeType, "TheDude", ricky, pops, ma);

	cout << "Ol'Pops: " << olpops << endl;
	cout << "Pops: " << pops << endl;
	cout << "Ma: " << ma << endl;
	cout << "Ricky: " << ricky << endl;
	cout << "TheDude: " << thedude << endl;

	Var* v = new_var(DudeType, "A");
	cout << "Var: " << v << endl;

	Var* nv = v->extend_attr("buddy");
	cout << "Extended: " << nv << endl;

	Var* nv2 = v->extend_attr("dad")->extend_attr("dad");
	cout << "Extended: " << nv2 << endl;

	cout << "Grandad: " << *nv2->apply_deref(thedude) << endl;
	
}

int main(){
	test_var();
}
