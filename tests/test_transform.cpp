#include "../include/helpers.h"
#include "../include/flattener.h"
#include "../include/vectorizer.h"
#include "test_macros.h"
#include <random>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <chrono>
using namespace std;
using namespace std::chrono;




// template <typename T>
// std::string vector_to_string(const std::vector<T>& vec) {


//     std::stringstream ss;
//     ss << "[" << std::fixed;
//     for (size_t i = 0; i < vec.size(); ++i) {
//         if (i != 0) {
//             ss << ", ";
//         }
//         ss << vec[i];
//     }
//     ss << "]";
//     return ss.str();
// }

void test_flattener(){
	FactType* CatType = define_fact("Cat", 
	    {{"id", cre_str, {{"unique_id", true}} },
	     {"color", cre_str, {{"visible", true}}},
	     {"legs", cre_int, {{"semantic", true}}},
	     {"frisky", cre_bool, {{"visible", false}}}
	 	}
	);

	Fact* snowball = make_fact(CatType, "snowball", "white", 3, false);
	FactSet* fs = new FactSet();
	FactSet* flat_fs;
	fs->declare(snowball);

	auto f = new Flattener(fs);
	flat_fs = f->apply(fs);
	IS_TRUE(f->get_member_inds(snowball->type)->size() == 3);
	IS_TRUE(flat_fs->size() == 3);
	cout << "F1:" << *f->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

	auto f2 = new Flattener(fs, {{"semantic", Item(true)}});
	flat_fs = f2->apply(fs);
	IS_TRUE(f2->get_member_inds(snowball->type)->size() == 1);
	IS_TRUE(flat_fs->size() == 1);
	cout << "F2:" << *f2->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

	auto f3 = new Flattener(fs, Flattener::default_flags, true);
	flat_fs = f3->apply(fs);
	// IS_TRUE(f->get_member_inds(snowball->type)->size() == 3);
	// IS_TRUE(flat_fs->size() == 3);
	cout << "F3:" << *f3->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

}



FactSet* random_cats(size_t N){
	FactType* CatType = define_fact("Cat", {
		 {"id", cre_int, {{"unique_id", true}} },
	     {"name", cre_str, {{"semantic", true}} },
	     {"color", cre_str, {{"visible", true}} },
	     {"legs", cre_int, {{"semantic", true}} },
	     {"frisky", cre_bool, {{"visible", false}} },
	     {"x", cre_float, {{"visible", true}} },
	     {"y", cre_float, {{"visible", true}} },
	 	}
	);

	// std::srand(77); 
	std::vector<std::string> names = {"fluffer", "soren", "sango", "snowball", "crabcake"};
	std::vector<std::string> colors = {"black", "white", "calico", "orange", "brown"};
	std::vector<bool> friskies = {true, false};

	size_t M = CatType->members.size();
	FactSetBuilder* fs_builder = new FactSetBuilder(N, N*SIZEOF_FACT(M));
	for(int i=0; i < N; i++){
		std::vector<Item> items = {
		 Item(i),
		 Item(names[std::rand()%names.size()]),
		 Item(colors[std::rand()%colors.size()]),
		 Item(4),
		 Item(friskies[std::rand()%friskies.size()]),
		 Item(rand_flt()),
		 Item(rand_flt())
		};
		fs_builder->add_fact(CatType, items);
	}
	return fs_builder->fact_set;
}


FactSet* setup_factset(size_t N, size_t M){
	FactType* BoopType = define_fact("Boop", 
	    {{"index", cre_str, {{"visible", true}} },
	     {"name", cre_str, {{"visible", true}}},
	     {"is_cool", cre_int, {{"visible", true}}}
	 	}
	);

	FactSetBuilder* fs_builder = new FactSetBuilder(N, N*(sizeof(Fact) + sizeof(Item)*M));
	std::vector<Item> items = {Item(0), Item("A"), Item(false)};
	for(int i=0; i < N; i++){
		FactSetBuilder_add_fact(fs_builder, BoopType, items.data(), items.size());
	}
	return fs_builder->fact_set;
}

void bench_flattener(){
	// FactSet* fs = setup_factset(10000, 3);
	FactSet* fs = random_cats(10000);
	// auto f = new Flattener(fs);
	time_it_n("_update_init", (new Flattener(fs))->_update_init();, 500);
}

void test_vectorizer(){
	FactSet* fs = random_cats(4);
	Flattener* flattener = new Flattener(fs);
	Vectorizer* vectorizer = new Vectorizer();

	auto flat_fs = flattener->apply(fs);
	cout << "FLAT STATE :" << endl << flat_fs->to_string() << endl;
	auto [nom_vec, flt_vec] = vectorizer->apply(flat_fs);
	cout << nom_vec << endl;
	cout << flt_vec << endl;

	fs = random_cats(4);
	flat_fs = flattener->apply(fs);
	cout << "FLAT STATE :" << endl << flat_fs->to_string() << endl;
	std::tie(nom_vec, flt_vec) = vectorizer->apply(flat_fs);
	cout << nom_vec << endl;
	cout << flt_vec << endl;
}

void bench_vectorizer(){

	size_t N = 10000;
	FactSet* fs = random_cats(N);
	auto flattener = new Flattener(fs);
	flattener->_update_init();
	FactSet* flat_fs = flattener->builder.fact_set;

	// for (auto it = flat_fs->begin(); it != flat_fs->end(); ++it) {
	// 	Fact* fact = *it;
	// 	cout << "Fact" << fact << endl;
	// 	cout << "HASH: " << CREHash{}(fact) << endl;
	// }
	// auto f = new Flattener(fs);
	// Vectorizer* vectorizer = new Vectorizer();
	// vectorizer->apply(flat_fs);
	// size_t buff_size = //10000*SIZEOF_FACT(3);
	time_it_n("vectorize new",(new Vectorizer(3*N))->apply(flat_fs); , 100);

	Vectorizer* vectorizer = new Vectorizer(3*N);
	vectorizer->apply(flat_fs);

	time_it_n("vectorize reuse",vectorizer->apply(flat_fs);, 100);
}


int main(){
	test_flattener();
	// bench_flattener();
	// test_vectorizer();
	// bench_vectorizer();
	return 0;
}