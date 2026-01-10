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

    ref<Var> A = new_var("A", CatType);
    ref<Var> B = new_var("B", CatType);

    ref<Logic> logic = AND(
        EqualsStr(A->extend_attr("color"), "brown"), GreaterThanInt(A->extend_attr("age"), 5),
        EqualsStr(B->extend_attr("color"), "white"), LessThanInt(B->extend_attr("age"), 5)
    );
    cout << "Logic: " << logic << endl;
    
    // Create a FactSet
    ref<FactSet> fact_set = new FactSet();

    fact_set->declare(make_fact(CatType, "snickers", "brown", 10));
    fact_set->declare(make_fact(CatType, "dingo", "brown", 2));
    fact_set->declare(make_fact(CatType, "fluffer", "white", 6));
    fact_set->declare(make_fact(CatType, "sango", "white", 4));
    
    // Create a CORGI_Graph
    CORGI_Graph graph;
    graph.fact_set = fact_set.get();
    // Initialize nodes_by_nargs to avoid out-of-bounds access
    graph.nodes_by_nargs.resize(10);
    
    // Add the logic to the graph
    graph.add_logic(logic.get());
    
    cout << "Graph nodes: " << graph.nodes.size() << endl;
    IS_TRUE(graph.nodes.size() > 0);

    graph.update();
    
    cout << "Test passed!" << endl;
}

int main(){
    test_corgi_basic();
    return 0;
}

