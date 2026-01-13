#pragma once
#include <memory>
#include "../include/ref.h"
#include "../include/factset.h"
#include "../include/literal.h"
#include "../include/func.h"
#include "../include/logic.h"
#include "../include/var_inds.h"
#include "../include/mapping.h"
#include "types.h"
#include "var.h"
#include <cstdint>
#include <map>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <span>
#include <cstddef>
#include <algorithm>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/src/Core/NumTraits.h>
#include <unsupported/Eigen/CXX11/Tensor>

namespace cre{
    struct CORGI_Node;
    struct CORGI_Graph;
    struct InputState;
    struct MatchIter;
}

namespace cre {

struct BitMatrix {
    uint32_t cap_row_bytes;
    uint32_t cap_col_bytes;
    uint32_t n_rows;
    uint32_t n_cols;
    uint8_t* data = nullptr;

    void reserve(uint32_t n_rows, uint32_t n_cols);
    void set_byte(uint32_t byte_row_ind, uint32_t byte_col_ind, uint8_t value);
    uint8_t get_byte(uint32_t byte_row_ind, uint32_t byte_col_ind) const;
    bool get(uint32_t row, uint32_t col) const;
    void set(uint32_t row, uint32_t col, bool value);
    BitMatrix(std::size_t n_rows, std::size_t n_cols);
    bool get(std::size_t row, std::size_t col) const;
};




struct InputEntryState {
    // The input fact's f_id
    int64_t f_id;

    // For beta nodes the number of facts paired with this one. 
    int64_t true_count = 0;

    // The index of input fact in the associated output.
    int64_t output_ind = -1;


    

    // Indicates that the input fact is not removed
    bool source_valid = false;

    // ---- Flags related to updating head changes ----

    // Some change to the underlying fact of this entry
    //  or an associated field and we need to re-check its head value.
    bool needs_head_check = true;

    // Indicates that the input fact is not removed and its relevant 
    //  attributes have been successfully dereferenced.
    bool head_is_valid = true;

    // Some change to the head values of this entry has occured
    //   and we'll need to re-evaluate any relations that depend on it.
    bool needs_update_relation = true;


    // ---- Flags for updating output changes ----

    // Indicates that there was ever a match for this input fact.
    bool true_ever_nonzero = false;

    // Indicates that the input fact was invalidated in this match cycle.
    // bool recently_invalid = false;

    // Indicates that the input fact was inserted in this match cycle.
    // bool recently_inserted = false;

    // Indicates that the input fact was matched in this match cycle.
    // bool recently_nonzero = false;

    // Indicates that the input fact is present in the output.
    bool present_in_output = false;

    // bool pad[3]; 

    
    

    

    // // The input fact was modified in this match cycle.
    // bool recently_modified = false;

    // // The input fact was removed or invalidated in this match cycle.
    // bool recently_invalid = false;

    

    // The input fact was matched in the previous match cycle 
    // bool true_was_nonzero = false;

    // The input fact has ever been a match. Used to keep track of 
    //  where holes should be kept in this input's corresponding output. 
    // bool true_ever_nonzero = false;
    // bool pad[1];

    InputEntryState(int64_t f_id);

    // void reset_temp_flags() {
    //     changed = false;
    //     recently_inserted = false;
    //     recently_modified = false;
    //     recently_invalid = false;
    // }
};



struct CORGI_IO {
    size_t _size = 0;
    
    // Indicies changed in this match cycle.
    // std::vector<int64_t> changed_inds = {};
    // Indicies removed in this match cycle.
    // std::vector<int64_t> remove_inds = {};

    // The set of f_ids held by this input/output.
    std::vector<int64_t> match_f_ids = {};
    // The indicies in the input of associated output facts.
    std::vector<int64_t> input_inds = {};

    // A mapping of idrecs to their indicies in the output.
    // std::map<uint64_t, int64_t> idrecs_to_inds = {};

    

    // A vector that keeps track indicies of holes in the output.  
    std::vector<int64_t> match_holes = {};
    // The size of the output including holes.  

    // int64_t width = 0;
    // A weak pointer to the node upstream to this one
    CORGI_Node* upstream_node = nullptr;
    int64_t arg_ind = -1;

    // A vector of input states that take this as an input.
    std::vector<InputState*> downstream_input_states = {};

    // bool is_root = false;
    CORGI_IO(CORGI_Node* upstream_node = nullptr, size_t arg_ind = -1);

    int64_t size() const;
    int64_t capacity() const;
    void downstream_signal_add(size_t ind, int64_t f_id);
    void downstream_signal_remove(size_t ind);
    int64_t add(int64_t f_id, int64_t input_ind = -1);
    void add_at(size_t ind, int64_t f_id, int64_t input_ind = -1);
    void remove_at(size_t ind);

    std::string to_string() const;
};

// struct DependsComparator {
//     /** Comparator over ChangeEvents.
//     */
//     bool operator()(const ChangeEvent& rec1, const ChangeEvent& rec2) const {
//         if(rec1.f_id !=  rec2.f_id) return rec1.f_id < rec2.f_id;
//         if(rec1.mbr_ind !=  rec2.mbr_ind) return rec1.mbr_ind < rec2.mbr_ind;
//         if(rec1.mbr_ind !=  rec2.mbr_ind) return rec1.mbr_ind < rec2.mbr_ind;
        
//     }
// };

struct HeadValueBuffer {
    size_t byte_width = 0;
    size_t n_heads = 0;
    size_t capacity = 0;
    void* data = nullptr;
    std::vector<size_t> head_offsets = {};

    HeadValueBuffer(Func* func, size_t arg_ind);
    HeadValueBuffer();
    void resize(size_t n_entries);

    inline void* get_entry_data_ptr(size_t ind) const {
        return (void*) ((uint8_t*) data + ind * byte_width);
    }
};

struct ChangeDep {
    // An input state in a node that depends on this change event.
    InputState* input_state;
    // The index of the input fact in the input state.
    uint64_t ind : 63;

    // Indicates that this dependency is part of a derefrence chain
    //  and not the final (i.e. head) dereference in the chain.
    bool is_deref_support : 1;

    ChangeDep(InputState* input_state, size_t ind, bool is_deref_support=false);
};

using ChangeDependsMap = std::map<ChangeEvent, std::vector<ChangeDep>>;
// using HeadPtrTensor = Eigen::Tensor<Item*, 2, Eigen::RowMajor>;
using HeadPtrTensor = Eigen::Matrix<Item*, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

//TODO: HeadsState
struct InputState {
    // A vector of idrecs that is filled directly in parse_change_events()
    //  on each match cycle to indicate a modify() relevant to this node.

    CORGI_Node* node;
    CORGI_IO* input;
    std::vector<Var*> head_var_ptrs;
    // std::vector<size_t> head_offsets;
    HeadRange head_range;
    size_t n_heads; 
    // std::span<HeadInfo> head_infos;

    size_t _capacity; 
    size_t _size; 
    std::vector<int64_t> modified_f_ids = {};
    
    HeadValueBuffer head_value_buffer={};
    std::vector<InputEntryState> entry_states = {};


    std::vector<int64_t> changed_inds = {};
    // std::vector<int64_t> c_buffer;
    // std::span<int64_t> changed_inds;
    // std::span<int64_t> unchanged_inds;
    // std::span<int64_t> removed_inds;



    size_t size();
    size_t capacity();
    InputState(CORGI_Node* node, size_t arg_ind);
    void signal_add(size_t ind, int64_t f_id);
    void signal_remove(size_t ind);
    void signal_modify(size_t ind, bool is_deref_support=false);
    void ensure_larger_than(size_t new_size, size_t new_capacity=-1);
    bool validate_head_or_retract(int64_t f_id, size_t change_ind);
    void insert_change_dep(int64_t f_id, 
                          const DerefInfo& deref_info,
                          int64_t ind=-1, bool is_deref_support=true);
    Item* resolve_head_ptr(int64_t f_id, Var* var, int64_t ind=-1);
    
    // void _check_modified_head(int64_t ind, InputEntryStat* e_state){
    //     bool was_valid = e_state->is_valid;
    //     bool is_valid = validate_head_or_retract(ind, ); // ??
    //     bool recently_inserted = (!was_valid & is_valid);
    //     bool recently_modified = (is_modify & is_valid);

    //     // Assign changes to the input_state struct.
    //     e_state->f_id = f_id;
    //     if(was_valid && !is_valid){
    //         e_state->recently_invalid = true;
    //     }
    //     e_state->recently_inserted = recently_inserted;
    //     e_state->recently_modified = is_valid;
    //     e_state->is_valid = is_valid;
    // }

    // void _resolve_head(int64_t ind, InputEntryStat* e_state){
    //     int64_t f_id = input->match_f_ids[ind];
    //     InputEntryState& e_state = entry_states[ind]; 

    //     bool was_valid = e_state->is_valid;

        
    //     bool is_valid = validate_head_or_retract(ind, ); // ??
    //     bool recently_inserted = (!was_valid & is_valid);
    //     // bool recently_modified = (is_modify & is_valid);

    //     // Assign changes to the input_state struct.
    //     e_state->f_id = f_id;
    //     if(was_valid && !is_valid){
    //         e_state->recently_invalid = true;
    //     }
    //     e_state->recently_inserted = recently_inserted;
    //     // e_state->recently_modified = recently_modified;
    //     e_state->is_valid = is_valid;

    // }

    // void _resolve_head(size_t change_ind, bool is_modify){
        // int64_t f_id = input->match_f_ids[change_ind];
        // InputEntryState& e_state = entry_states[change_ind]; 

        // bool was_valid = e_state.is_valid;
        
        // bool is_valid = validate_head_or_retract(f_id, change_ind);
        // bool recently_inserted = (!was_valid & is_valid);
        // bool recently_modified = (is_modify & is_valid);

        // Assign changes to the input_state struct.
        // e_state.f_id = f_id;
        // if(was_valid && !is_valid){
        //     e_state.recently_invalid = true;
        // }
        // e_state.recently_inserted = recently_inserted;
        // e_state.recently_modified = recently_modified;
        // e_state.is_valid = is_valid;

    // }
        


    void update();
};



                 
struct CORGI_Node {
    // Weak pointer to graph this node belongs to
    CORGI_Graph* graph;

    // Weak pointer to the working memory for this graph
    // FactSet* fact_set;

    // The Literal associated with this node
    ref<Literal> literal;

    // The Op for the node's literal
    // ref<Func> func;
    Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> 
      truth_table = {};
    
    
    // // The number of Vars (1 for alpha or 2 for beta)
    // size_t n_vars;

    // // The var_inds for the vars handled by this node 
    // VarInds var_inds;
    // t_ids
    std::vector<CORGI_IO*> inputs;
    std::vector<std::unique_ptr<CORGI_IO>> outputs;
    std::vector<InputState> input_states;

    bool upstream_same_parents = false;
    bool upstream_aligned = false;

    CORGI_Node(CORGI_Graph* graph, Literal* literal, const std::vector<CORGI_IO*>& inputs);
    void update_alpha_matches_func();
    void update_beta_matches_func();
    void update_output_changes();
    void update();


    template <class ... Ts>
    bool eval_func_relation(Func* func, Ts&& ... entry_inds){
        void** head_val_ptrs = (void**) alloca(sizeof(void**)*func->head_infos.size());
        void* ret_ptr = (void*) alloca(func->return_type->byte_width);
        int i = 0;
        ([&] {	
            HeadValueBuffer& head_value_buffer = input_states[i].head_value_buffer;
            size_t entry_ind = entry_inds; // Variatic expansion of entry_inds
            auto h_start = func->head_ranges[i].start;
            auto h_end = func->head_ranges[i].end;
            
            size_t j = 0;
            for(size_t head_ind=h_start; head_ind<h_end; ++head_ind){
                size_t head_offset = head_value_buffer.head_offsets[j];
                void* entry_data_ptr = head_value_buffer.get_entry_data_ptr(entry_ind);
                void* head_val_ptr = (void*) ((uint8_t*) entry_data_ptr + head_offset);
                
                head_val_ptrs[head_ind] = head_val_ptr;
                j++;
            }
            ++i;        
        } (), ...);

        func->call_recursive_fc(func, ret_ptr, head_val_ptrs);
        Item ret_item = func->ptr_to_item_func(ret_ptr);
        return ret_item.as<bool>();
    }
};

// std::vector<ChangeEvent> accumulated_change_events(
//     std::span<ChangeEvent> change_queue,
//     size_t start,
//     size_t end=-1
// ){
//     if(end == -1) end = change_queue.size();
//     std::vector<ChangeEvent> change_events;
//     // for(size_t i=start; i < end; i++){
//     //     change_events.push_back(change_queue[i]);
//     // }
//     // return change_events;
// }

// struct TypeInfo {
//     CRE_Type* type;
//     std::vector<CORGI_IO> root_inputs;
// }

struct VarFrontier {
    std::vector<Literal*> upstream_depends = {};
    CORGI_IO* output;

    VarFrontier(CORGI_IO* output);
};


using EndJoinPtrsMatrix = Eigen::Tensor<CORGI_Node*, 2, Eigen::RowMajor>;


struct LogicGraphView {
    // Weak pointer to the graph this view is of.
    CORGI_Graph* graph;

    // Weak pointer to the logic this view is for.
    Logic* logic;

    // Graph nodes for this Logic 
    std::vector<CORGI_Node*> nodes = {};

    // Graph nodes for this Logic organized by [[...alphas],[...betas],[...etc]]
    std::vector<std::vector<CORGI_Node*>> nodes_by_nargs = std::vector<std::vector<CORGI_Node*>>(8);
    std::vector<VarFrontier> frontiers;
    EndJoinPtrsMatrix end_join_ptrs; 

    MatchIter* match_iter_prototype = nullptr;

    void _add_literal(Literal* lit, std::vector<VarFrontier>& frontiers, EndJoinPtrsMatrix& end_join_ptrs);
    void _add_logic(Logic* logic, std::vector<VarFrontier>& frontiers, EndJoinPtrsMatrix& end_join_ptrs);

    LogicGraphView(CORGI_Graph* graph, Logic* logic);
};

struct CORGI_Graph {
    // The change_head of the working memory at the last graph update.
    size_t change_head = 0;

    // The working memory memset.
    FactSet* fact_set = nullptr;

    // Owning vector of nodes in this graph
    std::vector<std::unique_ptr<CORGI_Node>> nodes = {};

    // All logic views for this graph.
    std::vector<LogicGraphView> logic_views = {};



    // Owning List of root nodes (i.e. the nodes that holds all
    //  match candidates for a fact_type before filtering).
    // std::vector<std::unique_ptr<CORGI_Node>> root_nodes = {};

    // Owning vector of root inputs, one for each type index.
    std::vector<std::unique_ptr<CORGI_IO>> root_inputs = std::vector<std::unique_ptr<CORGI_IO>>(64);

    // // Maps type_indices to root inputs for facts of that type.
    // std::vector<std::vector<CORGI_IO*>> type_to_root_inputs = {};

    // Maps types to the root node outputs associated with facts of that type.
    // std::map<CRE_Type*, CORGI_IO*> root_map = {};

    // ???
    ChangeDependsMap change_dep_map = {};

    // Maps (t_id, 0, a_id) idrec patterns to (node,arg_ind) that should be 
    //  rechecked based on that pattern.
    // std::map<uint64_t, std::vector<std::pair<CORGI_Node*, size_t>>> global_modify_map;

    // The sum of weights of all literals in the graph
    // float total_structure_weight = 0.0f;
    // float total_match_weight = 0.0f;

    void _ensure_root_inputs(Logic* logic);
    std::vector<size_t> _get_degree_order(Logic* logic);
    
    void add_logic(Logic* logic);
    CORGI_IO* get_root_io(CRE_Type* type);
    void parse_change_events();
    void update();

    CORGI_Graph(FactSet* fact_set) : fact_set(fact_set) {}
};

// Forward declarations
struct MatchIterNode;

struct IterNodeDep {
    CORGI_Node* node;
    // Pointers to downstream nodes on which this m_node depends
    //  NOTE: the dependant node might not be the .node of the
    //  corresponding dependant m_node. This points to the most
    //  downstream join for a particular var pair. 
    MatchIterNode* m_node;

    // Index of a downstream m_node on which this iter node depends.
    size_t ind;

    // The arg_ind in each node in dep_node_ptrs for the var this
    //  m_node is associated with. 
    size_t arg_ind;

    IterNodeDep(CORGI_Node* node, MatchIterNode* m_node, size_t ind, size_t arg_ind) :
        node(node), m_node(m_node), ind(ind), arg_ind(arg_ind) {}
};


struct MatchIterNode {
    // The corgi node that this match iter node is associated with.
    CORGI_Node* node;

    // The arg_ind in the node that this match iter node is associated with.
    size_t arg_ind;

    // The var_ind within the Logic statement that this match iter node is associated with.
    size_t var_ind;
   
    // The index of this match iter node in the match iter.
    size_t m_node_ind;
    
    // The f_ids associated with this match iter node.
    std::vector<int64_t> f_ids = {};

    // The current index within f_ids for this iter node.
    int64_t curr_ind = -1;

    // The indicies of the downstream match iter nodes on which this match iter node depends.
    std::vector<IterNodeDep> upstream_deps = {};

    MatchIterNode(): node(nullptr), arg_ind(-1), var_ind(-1), m_node_ind(-1) {}

    MatchIterNode(CORGI_Node* node, size_t arg_ind, size_t var_ind, size_t m_node_ind) :
        node(node), arg_ind(arg_ind), var_ind(var_ind), m_node_ind(m_node_ind) {}
};

struct MatchIter {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = ref<Mapping>;
    using pointer           = ref<Mapping>*;
    using reference         = ref<Mapping>&;

    LogicGraphView* logic_view;
    CORGI_Graph* graph;
    Logic* logic;
    std::vector<MatchIterNode> m_nodes;
    ref<Mapping> curr_match;
    // std::vector<CRE_Type*> output_types;
    bool is_empty;
    bool iter_started;

    explicit MatchIter(LogicGraphView* logic_view) : 
        logic_view(logic_view), graph(logic_view->graph), logic(logic_view->logic) {

        m_nodes = {};
        m_nodes.resize(logic->vars.size() + 1);

        std::vector<MatchIterNode*> handled_vars = {};
        handled_vars.resize(logic->vars.size() + 1, nullptr);

        size_t n_vars = logic->vars.size();

        // INIT
        if(logic_view->match_iter_prototype == nullptr){

            // Loop downstream to upstream through the end nodes. Build a MatchIterNode
            //  for each end node to help us iterate over valid matches in the graph.
            // for var_ind in range(len(graph.end_nodes)-1,-1,-1):
            for(size_t i=0; i <= logic->vars.size(); i++){
                VarFrontier& frontier = logic_view->frontiers[i];
                m_nodes.emplace_back(
                    MatchIterNode(frontier.output->upstream_node, 
                        frontier.output->arg_ind, i, m_nodes.size())
                );
                MatchIterNode* m_node = &m_nodes.back();

                for(size_t j=0; j<n_vars; j++){
                    CORGI_Node* dep_node = logic_view->end_join_ptrs(i, j);
                    MatchIterNode* dep_m_node = handled_vars[j];
                    if(dep_node != nullptr && dep_m_node != nullptr){
                        // Find the arg_ind in the dep_node that corresponds to the var_ind
                        size_t arg_ind = -1;
                        for(size_t k=0; k<dep_node->literal->var_inds.size(); k++){
                            if(dep_node->literal->var_inds[k] == j){
                                arg_ind = k; break;
                            }
                        }
                        m_node->upstream_deps.emplace_back(
                            IterNodeDep(dep_node, dep_m_node, j, arg_ind)
                        );
                    }
                }

                // Mark this var as being handled by m_node.
                handled_vars[i] = m_node;
            }


        }

    }
    
    bool operator==(const MatchIter& other) const { 
        if(is_empty != other.is_empty) return false;
        if(m_nodes.size() != other.m_nodes.size()) return false;
        for(size_t i=0; i<m_nodes.size(); i++){
            if(m_nodes[i].node != other.m_nodes[i].node) return false;
            if(m_nodes[i].arg_ind != other.m_nodes[i].arg_ind) return false;
            if(m_nodes[i].curr_ind != other.m_nodes[i].curr_ind) return false;
        }
        return true;
    }
    bool operator!=(const MatchIter& other) const { 
        return !(*this == other);
    }
    auto& operator*() const { return *curr_match; }

    MatchIter& operator++() { 
        // Increment the m_iter nodes until satisfying all upstream
        while(!is_empty){
            int64_t most_upstream_overflow = -1;

            // On a fresh iterator skip incrementating and 
            //  just make sure upstream updates are applied
            if(!iter_started){
                most_upstream_overflow = 0;
                iter_started = true;

            // Otherwise increment from downstream to upstream
            }else{
                most_upstream_overflow = -1;
            
                // For each m_node from downstream to upstream
                for(int64_t i=m_nodes.size()-1; i >= 0; i--){
                    MatchIterNode& m_node = m_nodes[i];

                    // Increment curr_ind if m_node the most downstream or if
                    //  a downstream m_node overflowed.
                    if(i == m_nodes.size()-1 || most_upstream_overflow == i+1){
                        ++m_node.curr_ind;
                    }

                    // Track whether incrementing also overflowed the ith m_node.
                    if(m_node.curr_ind >= m_node.f_ids.size()){
                        m_node.curr_ind = 0;
                        most_upstream_overflow = i;
                    }
                }

                // If the 0th m_node overflows then iteration is finished.
                if(most_upstream_overflow == 0) is_empty = true;
            }
            // Starting with the most upstream overflow and moving downstream set m_node.f_ids
            //  to be the set of f_ids consistent with its upstream dependencies.
            bool idrec_sets_are_nonzero = true;
            for(size_t i=most_upstream_overflow; i<m_nodes.size(); i++){
                MatchIterNode& m_node = m_nodes[i];

                // Only Update if has dependencies
                if(m_node.upstream_deps.size() > 0) update_from_upstream_match(m_node);

                if(m_node.f_ids.size() == 0) idrec_sets_are_nonzero = false;
            }
            // If each m_node has a non-zero idrec set we can yield a match
            //  otherwise we need to keep iterating
            if(idrec_sets_are_nonzero) break;
        }

        // Fill in the matched f_ids
        for(int64_t i=m_nodes.size()-1; i >= 0; i--){
            MatchIterNode& m_node = m_nodes[i];
            int64_t f_id = m_node.f_ids[m_node.curr_ind];
            ref<Fact> fact = graph->fact_set->get(f_id);
            curr_match->set(i,Item(fact));
        }
        return *this; 
    }

    void update_from_upstream_match(MatchIterNode& m_node){
        bool multiple_deps = m_node.upstream_deps.size() > 1;
        std::map<int64_t, bool> f_id_set = {};

        for(size_t i=0; i<m_node.upstream_deps.size(); i++){
            // Each dep_node is the terminal beta node (i.e. a graph node not an iter node) 
            //  between the vars iterated by m_node and dep_m_node, and might not be the same
            //  as dep_m_node.node.
            IterNodeDep& dep = m_node.upstream_deps[i];
            int64_t arg_ind = dep.arg_ind == 0 ? 1 : 0;

            // Extract the idrec for the current fixed dependency value in dep_m_node
        }
        if(multiple_deps){
            for(size_t i=0; i<m_node.f_ids.size(); i++){

            }
        }

    }

};

// Node Update Phases
// 1) Parse Change Evernts: Process the input changes from the factset and pipe into input states.
// 2) Resolve the head pointers
// 3) Revaluate the relation along all changes or pairs of changes 
// 4) Update the output, and its changes 



} // namespace cre