#include <iostream>
#include <string>
#include "test_macros.h"
#include "../include/corgi.h"
#include "../include/logic.h"
#include "../include/factset.h"
#include "../include/builtin_funcs.h"
#include "../include/var.h"

using std::cout;
using std::endl;
using namespace cre;

void test_corgi_basic(){
    // Create a simple AND logic
    // Define Cat FactType
    FactType* CatType = define_fact("Cat", {
        {"name", cre_str},
        {"color", cre_str},
        {"age", cre_int},
    });

    // Create a FactSet
    ref<FactSet> fact_set = new FactSet();

    fact_set->declare(make_fact(CatType, "snickers", "brown", 10));
    fact_set->declare(make_fact(CatType, "dingo", "brown", 2));
    fact_set->declare(make_fact(CatType, "fluffer", "white", 6));
    fact_set->declare(make_fact(CatType, "sango", "white", 4));

    ref<Var> A = new_var("A", CatType);
    ref<Var> B = new_var("B", CatType);

    ref<Logic> logic = AND(
        EqualsStr(A->extend_attr("color"), "brown"), GreaterThanInt(A->extend_attr("age"), 5),
        EqualsStr(B->extend_attr("color"), "white"), LessThanInt(B->extend_attr("age"), 5)
    );

    cout << "--------" << endl;
    
    // CORGI_Graph graph(fact_set.get());
    // graph.add_logic(logic.get());
    // graph.update();

    ref<Logic> logic2 = AND(
        EqualsStr(A->extend_attr("color"), "brown"),
        EqualsStr(B->extend_attr("color"), "white"), 
          LessThanInt(A->extend_attr("age"), B->extend_attr("age"))
    );

    CORGI_Graph graph2(fact_set.get());
    graph2.add_logic(logic2.get());
    graph2.update();

}

int main(){
    test_corgi_basic();
    return 0;
}

