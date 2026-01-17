#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "test_macros.h"
#include "../include/corgi.h"
#include "../include/logic.h"
#include "../include/factset.h"
#include "../include/builtin_funcs.h"
#include "../include/var.h"
#include "../include/context.h"
#include "../include/types.h"
#include "../include/item.h"

using std::cout;
using std::endl;
using namespace cre;



// Helper function to extract match names from a MatchIter
std::vector<std::vector<std::string>> get_match_names(MatchIter* match_iter) {
    std::vector<std::vector<std::string>> result;
    
    if (match_iter == nullptr || match_iter->is_empty) {
        return result;
    }
    
    // Initialize the iterator
    // ++(*match_iter);
    
    while (!match_iter->is_empty) {
        ref<Mapping>& match = match_iter->curr_match;
        std::vector<std::string> match_names;
        
        // Extract names from each variable in the match
        for (size_t i = 0; i < match->size(); i++) {
            Item fact_item = match->get(i);
            if (fact_item.get_t_id() == T_ID_FACT) {
                Fact* fact = fact_item._as<Fact*>();
                Item name_item = fact->get("name");
                std::string_view name_sv = name_item.as<std::string_view>();
                match_names.push_back(std::string(name_sv));
            }
        }

        cout << "match_names: " << fmt::format("[{}]", fmt::join(match_names, ", ")) << endl;
        
        if (!match_names.empty()) {
            // std::sort(match_names.begin(), match_names.end());
            result.push_back(match_names);
        }
        
        ++(*match_iter);
    }
    
    // Sort the result
    // std::sort(result.begin(), result.end());
    
    return result;
}

bool matches_equal(const std::vector<std::vector<std::string>>& matches1, 
                   const std::vector<std::vector<std::string>>& matches2){
    if(matches1.size() != matches2.size()) return false;
    for(size_t i=0; i < matches1.size(); i++){
        if(matches1[i] != matches2[i]) return false;
    }
    return true;
}

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
    fact_set->declare(make_fact(CatType, "snowball", "white", 6));
    fact_set->declare(make_fact(CatType, "ol'fluffer", "white", 20));
    fact_set->declare(make_fact(CatType, "sango", "white", 4));

    ref<Var> A = new_var("A", CatType);
    ref<Var> B = new_var("B", CatType);

    ref<Logic> logic = AND(
        EqualsStr(A->extend_attr("color"), "brown"), GreaterThanInt(A->extend_attr("age"), 5),
        EqualsStr(B->extend_attr("color"), "white"), LessThanInt(B->extend_attr("age"), 5)
    );

    

    cout << "--------" << endl;
    
    CORGI_Graph graph(fact_set.get());
    graph.add_logic(logic.get());
    // graph.update();
    graph.logic_views[0].get_matches();

    cout << "--------" << endl;

    ref<Logic> logic2 = AND(
        EqualsStr(A->extend_attr("color"), "brown"),
        EqualsStr(B->extend_attr("color"), "white"), 
          LessThanInt(A->extend_attr("age"), B->extend_attr("age"))
    );

    CORGI_Graph graph2(fact_set.get());
    graph2.add_logic(logic2.get());
    // graph2.update();
    graph.logic_views[0].get_matches();

}




void test_same_parents(){
    
    // Define BOOP FactType
    FactType* BOOP = define_fact("BOOP", {
        {"name", cre_str},
        {"mod3", cre_float},
        {"mod5", cre_float},
        {"mod7", cre_float},
        {"val", cre_float}
    });
    
    // Create FactSet
    ref<FactSet> ms = new FactSet();
    
    // Declare 106 facts
    for (int i = 0; i < 106; i++) {
        std::string name = std::to_string(i);
        float mod3 = i % 3;
        float mod5 = i % 5;
        float mod7 = i % 7;
        float val = i;
        // cout << "declare: " << val << " m3:" << mod3 << " m5:" << mod5 << " m7:" << mod7 << endl;
        ms->declare(make_fact(BOOP, name, mod3, mod5, mod7, val));
    }
    
    // Create variables
    ref<Var> A = new_var("A", BOOP);
    ref<Var> B = new_var("B", BOOP);
    ref<Var> C = new_var("C", BOOP);
    
    // Aligned case
    ref<Logic> conds1 = AND(
        LessThan(A->extend_attr("val"), B->extend_attr("val")),
        Equals(A->extend_attr("mod3"), B->extend_attr("mod3")),
        Equals(A->extend_attr("mod5"), B->extend_attr("mod5")),
        Equals(A->extend_attr("mod7"), B->extend_attr("mod7"))
    );
    
    
    // CORGI_Graph graph1(ms.get());
    // graph1.add_logic(conds1.get());
    // ref<MatchIter> match_iter1 = graph1.logic_views[0].get_matches();
    ref<MatchIter> match_iter1 = conds1->get_matches(ms.get());
    std::vector<std::vector<std::string>> matches1 = get_match_names(match_iter1);
    IS_TRUE(matches_equal(matches1, {{"0", "105"}}));

    cout << "--------------" << endl;
    
    // Unaligned case
    ref<Logic> conds2 = AND(
        LessThan(A->extend_attr("val"), B->extend_attr("val")),
        Equals(B->extend_attr("mod3"), A->extend_attr("mod3")),
        Equals(A->extend_attr("mod5"), B->extend_attr("mod5")),
        Equals(B->extend_attr("mod7"), A->extend_attr("mod7"))
    );
    
    // CORGI_Graph graph2(ms.get());
    // graph2.add_logic(conds2.get());
    // ref<MatchIter> match_iter2 = graph2.logic_views[0].get_matches();
    ref<MatchIter> match_iter2 = conds2->get_matches(ms.get());
    std::vector<std::vector<std::string>> matches2 = get_match_names(match_iter2);
    IS_TRUE(matches_equal(matches2, {{"0", "105"}}));
    cout << "--------------" << endl;
    
    // Complex case with three variables
    ref<Logic> conds3 = AND(
        A, B, C,
        LessThan(A->extend_attr("val"), B->extend_attr("val")),
        Equals(B->extend_attr("mod3"), A->extend_attr("mod3")),
        Equals(A->extend_attr("mod5"), B->extend_attr("mod5")),
        Equals(B->extend_attr("mod7"), A->extend_attr("mod7")),
        LessThan(A->extend_attr("val"), C->extend_attr("val")),
        Equals(C->extend_attr("mod3"), A->extend_attr("mod7")),
        Equals(C->extend_attr("mod3"), B->extend_attr("mod5")),
        Equals(C->extend_attr("mod3"), A->extend_attr("mod7")),
        LessThan(C->extend_attr("val"), 12.0)
    );
    
    // CORGI_Graph graph3(ms.get());
    // graph3.add_logic(conds3.get());
    // ref<MatchIter> match_iter3 = graph3.logic_views[0].get_matches();
    ref<MatchIter> match_iter3 = conds3->get_matches(ms.get());
    std::vector<std::vector<std::string>> matches3 = get_match_names(match_iter3);
    IS_TRUE(matches_equal(matches3, 
        {{"0", "105", "3"}, {"0", "105", "6"}, {"0", "105", "9"}}));
    
}

int main(){
    // test_corgi_basic();
    test_same_parents();
    return 0;
}

