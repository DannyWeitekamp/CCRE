#pragma once

#include <cstddef>
// #include "../include/logic.h"
// #include "../include/factset.h"

namespace cre {
struct FactSet;
struct Logic;
struct BaseLogicView;
}

namespace cre {

struct BaseMatcherGraph {
        // The FactSet on that this graph matches objects of.
        FactSet* fact_set = nullptr;

        // The change_head of the working memory at the last graph update.
        size_t change_head = 0;

        virtual ~BaseMatcherGraph() = default;

        BaseMatcherGraph(FactSet* fact_set) : fact_set(fact_set) {}
};

struct BaseLogicView {
    // Weak pointer to the graph this view is of.
    BaseMatcherGraph* graph;

    // Weak pointer to the logic this view is for.
    Logic* logic;
};

// LogicGraphView* _resolve_logic_view(Logic* logic, FactSet* fact_set);

// template<typename T>
// BaseLogicView* _resolve_logic_view(Logic* logic, FactSet* fact_set){
//     // Fast path recycle the logic view cached in the logic object.
//     if(logic->matcher_view != nullptr){
//         BaseLogicView* lv = logic->matcher_view;
//         if(lv->graph->fact_set == fact_set){
//             return lv;
//         }
//     }

//     // Create a new graph if one doesn't exist.
//     if(fact_set->matcher_graph == nullptr){
//         // fact_set->matcher_graph = std::make_unique<CORGI_Graph>(fact_set);
//         fact_set->matcher_graph = new T(fact_set);
//     }
//     T* graph = (T*) fact_set->matcher_graph;

//     // Add the logic to the graph and cache the logic view in the logic object.
//     //   Note: add_logic can return the cached logic view if it already exists.
//     BaseLogicView* logic_view = (BaseLogicView*) graph->add_logic(logic);
//     logic->matcher_view = logic_view;
//     return logic_view;
// }

}