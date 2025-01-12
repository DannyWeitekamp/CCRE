#include "../include/helpers.h"
#include "../include/flattener.h"
#include "../include/vectorizer.h"
#include "test_macros.h"
#include <random>

#include <fmt/format.h>
#include <fmt/ranges.h>

// #include <chrono>
// using namespace std;
// using namespace std::chrono;

using std::cout;
using std::endl;
using namespace cre;



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

	ref<Fact> snowball = make_fact(CatType, "snowball", "white", 3, false);
	ref<FactSet> fs = new FactSet();
	fs->declare(snowball);

	cout << "INP:\n" << fs <<endl;

	ref<Flattener> f = new Flattener(fs);
	ref<FactSet> flat_fs = f->apply(fs);
	IS_TRUE(f->get_member_inds(snowball->type)->size() == 3);
	IS_TRUE(flat_fs->size() == 3);
	cout << "F1:" << *f->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

	cout << "INP:\n" << fs <<endl;

	ref<Flattener> f2 = new Flattener(fs, {{"semantic", Item(true)}});
	flat_fs = f2->apply(fs);
	IS_TRUE(f2->get_member_inds(snowball->type)->size() == 1);
	IS_TRUE(flat_fs->size() == 1);
	cout << "F2:" << *f2->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

	ref<Flattener> f3 = new Flattener(fs, Flattener::default_flags, true);
	flat_fs = f3->apply(fs);
	// IS_TRUE(f->get_member_inds(snowball->type)->size() == 3);
	// IS_TRUE(flat_fs->size() == 3);
	cout << "F3:" << *f3->get_member_inds(snowball->type) << endl;
	cout << flat_fs->to_string() << endl;

}



ref<FactSet> random_cats(size_t N){

	FactType* CatType = define_fact("Cat", {
		 {"id", cre_int, {{"unique_id", true}} },
	     {"name", cre_str, {{"visible", true}} },
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
	FactSetBuilder fs_builder = FactSetBuilder(N, N*SIZEOF_FACT(M));
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
		fs_builder.new_fact(CatType, items);
	}
	return fs_builder.fact_set;
}


ref<FactSet> setup_factset(size_t N){
	FactType* BoopType = define_fact("Boop", 
	    {{"index", cre_str, {{"visible", true}} },
	     {"name", cre_str, {{"visible", true}}},
	     {"is_cool", cre_int, {{"visible", true}}}
	 	}
	);

	FactSetBuilder fs_builder = FactSetBuilder(N, N*SIZEOF_FACT(3));
	std::vector<Item> items = {Item(0), Item("A"), Item(false)};
	for(int i=0; i < N; i++){
		fs_builder.new_fact(BoopType, items.data(), items.size());
	}
	return fs_builder.fact_set;
}

void bench_flattener(size_t N = 1000, size_t reps = 500){
	// ref<FactSet> fs = setup_factset(10000, 3);
	ref<FactSet> fs = random_cats(N);
	ref<Flattener> f = new Flattener(fs);
	time_it_n("flattener apply (wme)", (f->apply(fs));, reps);

	f = new Flattener(fs, true);
	time_it_n("flattener apply (var)", (f->apply(fs));, reps);
}

void test_vectorizer(){
	ref<FactSet> fs = random_cats(4);
	ref<Flattener> flattener = new Flattener(fs);
	ref<Vectorizer> vectorizer = new Vectorizer();

	auto flat_fs = flattener->apply(fs);
	cout << "FLAT STATE :" << endl << flat_fs->to_string() << endl;
	auto [nom_vec_p, flt_vec_p] = vectorizer->apply(flat_fs);
	auto& nom_vec = *nom_vec_p; 
	auto& flt_vec = *flt_vec_p; 

	cout << nom_vec << endl;
	cout << flt_vec << endl;

	fs = random_cats(4);
	flat_fs = flattener->apply(fs);
	cout << "FLAT STATE :" << endl << flat_fs->to_string() << endl;
	std::tie(nom_vec_p, flt_vec_p) = vectorizer->apply(flat_fs);
	nom_vec = *nom_vec_p; 
	flt_vec = *flt_vec_p; 

	cout << nom_vec << endl;
	cout << flt_vec << endl;

	for(int i=0; i < nom_vec.size(); i++){
		auto inverse = vectorizer->invert(i, nom_vec[i]);
		cout << "nv[" << i << "]=" << nom_vec[i] << "  " << inverse << endl;
	}

	for(int i=0; i < flt_vec.size(); i++){
		auto inverse = vectorizer->invert(i, flt_vec[i]);
		cout << "fv[" << i << "]=" << flt_vec[i] << "  " << inverse << endl;
	}
}

void bench_vectorizer(size_t N = 1000, size_t reps=500){
	
	ref<FactSet> fs = random_cats(N);
	
	// --- WME-Style use_vars=False----
	ref<Flattener> flattener = new Flattener(fs);
	flattener->_update_init();
	ref<FactSet> flat_fs = flattener->builder.fact_set;

	ref<Vectorizer> vectorizer = new Vectorizer(3*N);
	time_it_n("vectorize new (wme)",(vectorizer=new Vectorizer(3*N))->apply(flat_fs); , reps);

	
	vectorizer->apply(flat_fs);

	time_it_n("vectorize reuse (wme)",vectorizer->apply(flat_fs);, reps);

	// --- W/ use_vars=True ----
	flattener = new Flattener(fs, true);
	flattener->_update_init();
	flat_fs = flattener->builder.fact_set;

	vectorizer = new Vectorizer(3*N);

	time_it_n("vectorize new (vars)",(vectorizer=new Vectorizer(3*N))->apply(flat_fs); , reps);

	
	vectorizer->apply(flat_fs);

	time_it_n("vectorize reuse (vars)",vectorizer->apply(flat_fs);, reps);
}


// #include <type_traits>




int main(){
	// cout << FACT_ALIGN_IS_POW2 << " <IS ALIGN 2" << endl;
	// cout << FACT_NEED_ALIGN_PAD << " <NEED ALIGN PAD" << endl;
	// cout << "ALIGN PAD:" << ALIGN_PADDING(SIZEOF_FACT(0)) << endl;
	// cout << "ALIGN PAD:" << ALIGN_PADDING(SIZEOF_FACT(1)) << endl;
	// cout << "ALIGN PAD:" << ALIGN_PADDING(SIZEOF_FACT(2)) << endl;
	// cout << "ALIGN PAD:" << ALIGN_PADDING(SIZEOF_FACT(3)) << endl;
	// cout << "ALIGN PAD:" << ALIGN_PADDING(SIZEOF_FACT(4)) << endl;
	// cout << "ALIGN:" << alignof(Fact) << endl;
	// cout << "SIZE:" << SIZEOF_FACT(0) << endl;
	// cout << "SIZE:" << SIZEOF_FACT(1) << endl;
	// cout << "SIZE:" << SIZEOF_FACT(2) << endl;
	// cout << "SIZE:" << SIZEOF_FACT(3) << endl;
	// cout << "SIZE:" << SIZEOF_FACT(4) << endl;


	test_flattener();
	bench_flattener(10, 1);
	// bench_flattener(1000, 500);
	test_vectorizer();
	bench_vectorizer(10, 1);
	// bench_vectorizer(1000, 500);
	return 0;
}
