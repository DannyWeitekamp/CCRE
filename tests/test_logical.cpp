// Program to calculate the sum of n numbers entered by the user
#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <bit>
#include <bitset>
#include <cstdint>
#include <cstring>

#include "test_macros.h"

using std::cout;
using std::endl;

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

	ref<Fact> olpops = make_fact(DudeType, "Ol'Pops");
	ref<Fact> pops = make_fact(DudeType, "Pops", nullptr, olpops, nullptr);
	ref<Fact> ma = make_fact(DudeType, "Ma", pops);
	ref<Fact> ricky = make_fact(DudeType, "Ricky");
	ref<Fact> thedude = make_fact(DudeType, "TheDude", ricky, pops, ma);

	cout << "Ol'Pops: " << olpops << endl;
	cout << "Pops: " << pops << endl;
	cout << "Ma: " << ma << endl;
	cout << "Ricky: " << ricky << endl;
	cout << "TheDude: " << thedude << endl;

	ref<Var> v = new_var("A", DudeType);
	cout << "Var: " << v << ", hash=" << CREHash{}(v) << endl;

	ref<Var> nv = v->extend_attr("buddy");
	cout << "Extended: " << nv << ", hash=" << CREHash{}(nv) << endl;

	ref<Var> nv2 = v->extend_attr("dad")->extend_attr("dad");
	cout << "Extended: " << nv2  << ", hash=" << CREHash{}(nv2) << endl;

	cout << "Grandad: " << *nv2->apply_deref(thedude) << endl;

	ref<Var> a = v->extend_attr("dad");//->extend_attr("buddy");
	ref<Var> b = v->extend_attr("buddy");//->extend_attr("dad");
	a = a->extend_attr("buddy");
	b = b->extend_attr("dad");

	cout << v << ", " << nv2 << endl;
	cout << a << ", " << b << endl;
	cout << std::bitset<64>(CREHash{}(a)) << "\n" << std::bitset<64>(CREHash{}(b)) << endl;
	IS_TRUE(std::bitset<64>(CREHash{}(a)) != std::bitset<64>(CREHash{}(b)));
}

int main(){
	test_var();
}
