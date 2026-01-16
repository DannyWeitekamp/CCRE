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
    struct UpstreamIndexCache;
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
    
    // UpstreamIndexCache objects that depend on this IO.
    std::vector<UpstreamIndexCache*> upstr_index_caches = {};


    // bool is_root = false;
    CORGI_IO(CORGI_Node* upstream_node = nullptr, size_t arg_ind = -1);

    size_t size() const;
    size_t capacity() const;
    void downstream_signal_add(size_t ind, int64_t f_id);
    void downstream_signal_remove(size_t ind);
    int64_t add(int64_t f_id, int64_t input_ind = -1);
    void add_at(size_t ind, int64_t f_id, int64_t input_ind = -1);
    void remove_at(size_t ind);

    std::string to_string(bool verbose=false) const;
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
    void ensure_larger_than(size_t new_size);
    void ensure_larger_than(size_t new_size, size_t new_capacity);
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
    std::vector<CORGI_IO> outputs; // Owning vector of outputs (not a ptr)
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
        // cout << "func: ";
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
                // cout << *((double*) head_val_ptr) << " "; // head_val_ptr
                j++;
            }
            ++i;        
        } (), ...);

        

        func->call_recursive_fc(func, ret_ptr, head_val_ptrs);
        Item ret_item = func->ptr_to_item_func(ret_ptr);
        // cout << "->" << ret_item.as<bool>();
        // cout << endl;
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



// For each var X associated with a beta frontier node find the set of 
//  upstream end_join nodes A,B,C,... that need to be cross-referenced 
//  to determine if an assignement X->xi is consistent with all upstream 
//  assignments A->ai, B->bi, C->ci,... .To facilitate this we determine 
//  the path from X back to the most upstream end_join node on which it 
//  depends. We can used this to determine the index of an assignment X->xi 
//  within the dependent upstream end_join node. Resolving the upstream 
//  indicies lets quickly check the upstream node's truth table to determine 
//  if xi is consistent with the upstream assignments ai, bi, ci,... .
//  In principle traversing backward to resolve the upstream indicies 
//  as needed should be faster than maintaining an f_id -> index map in each node. 

struct UpstreamIndexCache {
    std::vector<size_t> upstream_var_inds = {};
    std::vector<std::tuple<CORGI_IO*, CORGI_Node*>> io_path = {};
    Eigen::Matrix<int64_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> 
       upstream_inds = {};

    UpstreamIndexCache(CORGI_IO* end_output, EndJoinPtrsMatrix& end_join_ptrs);
    void resolve_upstream_inds(size_t index_in_end_io);
    std::string to_string() const;

    int64_t index_of_node(CORGI_Node* node){
        size_t k = 0;
        for(auto& [io, _node] : io_path){
            // if(_node != nullptr){
            //     cout << "trying node: " << _node->literal->to_string() << ", " << k << endl;
            // }
            if(_node == node) return k;
            if(_node != nullptr) ++k;
        }
        return -1;
    }

    void resize(size_t n_entries){
        // cout << "RESIZE UPSTREAM IND CACHE: " << n_entries << ", " << upstream_var_inds.size() << endl;
        size_t n_rows = upstream_inds.rows();
        if(n_rows < n_entries) {
            size_t new_rows = std::max(n_entries, n_rows + 64);
            upstream_inds.conservativeResize(new_rows, upstream_var_inds.size());
            for(size_t i=n_rows; i < new_rows; i++){
                for(size_t j=0; j < upstream_var_inds.size(); j++){
                    upstream_inds(i, j) = -1;
                }
            }
        }
    }
};


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
    std::vector<UpstreamIndexCache> upstr_index_caches = {};

    std::unique_ptr<MatchIter> match_iter_prototype = nullptr;

    void _add_literal(Literal* lit, std::vector<VarFrontier>& frontiers, EndJoinPtrsMatrix& end_join_ptrs);
    void _add_logic(Logic* logic, std::vector<VarFrontier>& frontiers, EndJoinPtrsMatrix& end_join_ptrs);

    LogicGraphView(CORGI_Graph* graph, Logic* logic);

    MatchIter* get_matches();
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
    
    // The index cache for the dependant variable.
    UpstreamIndexCache* dep_index_cache;
    // size_t dep_cache_ind

    // The entry index in the cache for the dependant variable.
    size_t dep_cache_ind;
    size_t this_cache_ind;

    IterNodeDep(CORGI_Node* node, MatchIterNode* m_node,
               size_t ind, size_t arg_ind, 
               UpstreamIndexCache* dep_index_cache, size_t dep_cache_ind, size_t this_cache_ind) :
        node(node), m_node(m_node), 
        ind(ind), arg_ind(arg_ind), 
        dep_index_cache(dep_index_cache), 
        dep_cache_ind(dep_cache_ind),
        this_cache_ind(this_cache_ind) {}
};


struct MatchIterNode {
    // The corgi node that this match iter node is associated with.
    CORGI_Node* node;

    CORGI_IO* output;

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

    // The index cache for the variable node.
    UpstreamIndexCache* upstr_index_cache;

    bool is_empty = true;


    MatchIterNode(): node(nullptr), arg_ind(-1), var_ind(-1), m_node_ind(-1) {}

    MatchIterNode(CORGI_Node* node, size_t arg_ind, size_t var_ind,
        size_t m_node_ind, UpstreamIndexCache* upstr_index_cache) :
        node(node), arg_ind(arg_ind), var_ind(var_ind),
        m_node_ind(m_node_ind), upstr_index_cache(upstr_index_cache) {
            output = &node->outputs[arg_ind];
            f_ids.reserve(output->match_f_ids.size());
        }
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
    // bool iter_started;

    explicit MatchIter(LogicGraphView* logic_view) : 
        logic_view(logic_view), graph(logic_view->graph), logic(logic_view->logic) {

        // cout << "MatchIter constructor" << endl;
        
        size_t n_vars = logic->vars.size();
        m_nodes = {};
        m_nodes.reserve(n_vars);

        std::vector<MatchIterNode*> handled_vars = {};
        handled_vars.resize(n_vars, nullptr);

        // Loop downstream to upstream through the end nodes. Build a MatchIterNode
        //  for each end node to help us iterate over valid matches in the graph.
        // for var_ind in range(len(graph.end_nodes)-1,-1,-1):
        for(size_t i=0; i < n_vars; i++){
            VarFrontier& frontier = logic_view->frontiers[i];
            UpstreamIndexCache* this_index_cache = &logic_view->upstr_index_caches[i];
            m_nodes.emplace_back(
                MatchIterNode(frontier.output->upstream_node, 
                    frontier.output->arg_ind, i, m_nodes.size(), this_index_cache)
            );
            MatchIterNode* m_node = &m_nodes.back();
            cout << "!m_node: " << m_node->node->literal->to_string() << endl;
            

            for(size_t j=0; j < n_vars; j++){
                // if(i == j) continue;
                
                // cout << logic_view->end_join_ptrs << endl;
                
                CORGI_Node* dep_node = logic_view->end_join_ptrs(i, j);
                MatchIterNode* dep_m_node = handled_vars[j];

                // cout << "i: " << i << ", j: " << j << endl;
                // cout << "end_join_ptrs: " << endl;
                // cout << dep_node << endl;
                // cout << "---- " << endl;
                if(dep_node != nullptr && dep_m_node != nullptr){
                    // Find the arg_ind in the dep_node that corresponds to the var_ind
                    size_t arg_ind = -1;
                    for(size_t k=0; k < dep_node->literal->var_inds.size(); k++){
                        if(dep_node->literal->var_inds[k] == j){
                            arg_ind = k; break;
                        }
                    }
                    UpstreamIndexCache* dep_index_cache = &logic_view->upstr_index_caches[j];
                    int64_t dep_cache_ind = dep_index_cache->index_of_node(dep_node);
                    int64_t this_cache_ind = this_index_cache->index_of_node(dep_node);
                    cout << "  dep node: " << dep_node->literal->to_string() << endl;
                    cout << "    dep_cache_ind: " << dep_cache_ind << endl;
                    cout << "    this_cache_ind: " << this_cache_ind << endl;
                    assert(dep_cache_ind != -1);
                    m_node->upstream_deps.emplace_back(
                        IterNodeDep(dep_node, dep_m_node, j, arg_ind, dep_index_cache, dep_cache_ind, this_cache_ind)
                    );
                    // cout << "add dep " << "i: " << i << ", j: " << j << endl;
                }
            }
            // Mark this var as being handled by m_node.
            handled_vars[i] = m_node;
        }

        curr_match = new_mapping(n_vars, 
            logic_view->logic,
            &logic_view->logic->var_map);
        // cout << "curr_match: " << uint64_t(curr_match.get()) << endl;



        reset();

        // for(auto m_node : m_nodes){
        //     cout << "m_node: " << m_node.node->literal->to_string() << endl;
        //     cout << "m_node.f_ids: " << m_node.f_ids << endl;
        //     cout << endl;
        // }

        
    }

    void reset(){
        bool all_not_empty = reset_downstream_from(0);
        is_empty = !all_not_empty;

        // cout << "is_empty: " << is_empty << endl;
        if(!is_empty){
            fill_match();
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

    bool reset_downstream_from(size_t ind){
        assert(ind < m_nodes.size());

        // cout << "reset_downstream_from: " << ind << "," << m_nodes.size() <<  endl;
        // Starting with the most upstream overflow and moving downstream set m_node.f_ids
        //  to be the set of f_ids consistent with its upstream dependencies.
        bool all_not_empty = true;
        for(size_t i=ind; i<m_nodes.size(); i++){
            MatchIterNode& m_node = m_nodes[i];

            // Only Update if has dependencies
            // cout << i << " N deps: " << m_node.upstream_deps.size() << endl;
            update_from_upstream_match(m_node);

            // cout << i << " f_ids: " << m_node.f_ids << endl;

            if(m_node.is_empty){
                all_not_empty = false;
                break;
            }
        }
        return all_not_empty;
    }

    void fill_match(){
        // cout << "fill_match" << endl;
        // Fill in the matched f_ids 
        for(int64_t i=m_nodes.size()-1; i >= 0; i--){
            // cout << "i: " << i << endl;
            MatchIterNode& m_node = m_nodes[i];
            // cout << "   curr_ind: " << m_node.curr_ind << endl;
            assert(m_node.curr_ind >= 0);
            int64_t f_id = m_node.f_ids[m_node.curr_ind];
            // cout << "   f_id: " << f_id << endl;
            ref<Fact> fact = graph->fact_set->get(f_id);
            // cout << "   set fact: " << fact->to_string() << endl;
            curr_match->set(i,Item(fact));
        }
    }

    MatchIter& operator++() { 
        // Increment the m_iter nodes until satisfying all upstream
        while(!is_empty){
            // For each m_node from downstream to upstream
            int64_t most_upstream_overflow = -1;
            for(int64_t i=m_nodes.size()-1; i >= 0; i--){
                MatchIterNode& m_node = m_nodes[i];

                // Increment curr_ind if m_node is the last m_node or if
                //  an upstream m_node overflowed.
                if(i == m_nodes.size()-1 || most_upstream_overflow == i+1){
                    ++m_node.curr_ind;
                    while(m_node.curr_ind < m_node.f_ids.size() && 
                          m_node.f_ids[m_node.curr_ind] == -1){
                        ++m_node.curr_ind;
                    }
                }

                // Track whether incrementing also overflowed the ith m_node.
                if(m_node.curr_ind >= m_node.f_ids.size()){
                    m_node.curr_ind = 0;
                    most_upstream_overflow = i;
                }
            }

            // If the 0th m_node overflows then iteration is finished.
            if(most_upstream_overflow == 0){
                is_empty = true; return *this;
            }
            
            // Starting with the most upstream overflow and moving downstream set m_node.f_ids
            //  to be the set of f_ids consistent with its upstream dependencies.
            bool all_not_empty = true;
            if(most_upstream_overflow < m_nodes.size()){
                all_not_empty = reset_downstream_from(most_upstream_overflow);
            }
            
            // If each m_node has a non-zero f_id set we can yield a match
            //  otherwise we need to keep iterating
            if(all_not_empty) break;
        }

        fill_match();
        return *this; 
    }

    void update_from_upstream_match(MatchIterNode& m_node){
        // bool multiple_deps = m_node.upstream_deps.size() > 1;
        // std::map<int64_t, bool> f_id_set = {};

        // size_t width = m_node.output->match_f_ids.size();
        // bool*  match_mask = (bool*) alloca(sizeof(bool) * width);
        // memset(match_mask, true, sizeof(bool) * width);
        
        // Copy the match f_ids from the output to the m_node.
        m_node.is_empty = false;
        m_node.f_ids.clear();
        std::copy(m_node.output->match_f_ids.begin(),
                  m_node.output->match_f_ids.end(),// + m_node.output->size(),
                  std::back_inserter(m_node.f_ids));
        
        
        cout << endl;
        cout << "start f_ids: " << m_node.output->match_f_ids << endl;
        // cout << "curr ind: " << m_node.curr_ind << endl;
        cout << "This: " << m_node.node->literal->to_string() << endl;

        // Go through each upstream dependency check if each f_id
        // is consistent with the fixed f_ids upstream, if not set the f_id to -1.
        
        for(size_t i=0; i<m_node.upstream_deps.size(); i++){
            // Each dep_node is the terminal beta node (i.e. a graph node not an iter node) 
            //  between the vars iterated by m_node and dep_m_node, and might not be the same
            //  as dep_m_node.node.
            IterNodeDep& dep = m_node.upstream_deps[i];
            MatchIterNode* dep_m_node = dep.m_node;
            CORGI_Node* dep_node = dep.node;
            int64_t arg_ind = dep.arg_ind == 0 ? 1 : 0;

            cout << "   dep_node: " << dep_node->literal->to_string();
            cout << " f_ids: " << dep_m_node->f_ids << "curr_ind: " << dep_m_node->curr_ind << endl;

            // cout << "dep_m_node->curr_ind: " << dep_m_node->curr_ind << endl;

            // Get the index of curr_ind in the output handled by dep_m_node.
            assert(dep_m_node->curr_ind >= 0);
            // if(dep_m_node->curr_ind < 0) return;
            int64_t dep_ind = dep.dep_index_cache->upstream_inds(dep_m_node->curr_ind, dep.dep_cache_ind);
            
            for(size_t j=0; j<m_node.f_ids.size(); j++){

                // cout << m_node.upstr_index_cache->upstream_inds << endl;
                int64_t this_ind = m_node.upstr_index_cache->upstream_inds(j, dep.this_cache_ind);

                cout << "this_ind: " << this_ind << " dep_ind: " << dep_ind << endl;
                cout << "this_f_id: " << m_node.f_ids[j] << " dep_f_id: " << dep_m_node->f_ids[dep_ind] << endl;

                // Consult the dep_node's truth table to check if the f_id is 
                //   consistent with the current f_id in the dep_m_node.
                bool is_match = true;
                if(arg_ind == 0){
                    is_match = dep_node->truth_table(this_ind, dep_ind);
                }else{
                    is_match = dep_node->truth_table(dep_ind, this_ind);
                }

                cout << "is_match: " << is_match << endl;
                
                if(not is_match) m_node.f_ids[j] = -1;
            }
        }

        cout << "m_node f_ids: " << m_node.f_ids << endl;

        // Set the curr_ind to the first non-zero f_id
        for(size_t j=0; j<m_node.f_ids.size(); j++){
            if(m_node.f_ids[j] != -1){
                m_node.curr_ind = j;
                break;
            }
        }
        size_t max_valid_ind = -1;
        for(int64_t j=m_node.f_ids.size()-1; j>=0; --j){
            if(m_node.f_ids[j] != -1){
                max_valid_ind = j;
                break;
            }
        }
        m_node.f_ids.resize(max_valid_ind + 1);
        if(m_node.f_ids.size() == 0 ||
           m_node.f_ids[m_node.curr_ind] == -1){
            m_node.is_empty = true;
        }

        cout << "is_empty: " << m_node.is_empty << endl;
    }
};

// Node Update Phases
// 1) Parse Change Evernts: Process the input changes from the factset and pipe into input states.
// 2) Resolve the head pointers
// 3) Revaluate the relation along all changes or pairs of changes 
// 4) Update the output, and its changes 



} // namespace cre