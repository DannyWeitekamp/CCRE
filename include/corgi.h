#pragma once

#include "../include/ref.h"
#include "../include/factset.h"
#include "../include/literal.h"
#include "../include/func.h"
#include "../include/var_inds.h"
#include "types.h"
#include "var.h"
#include <cstdint>
#include <map>
#include <stdexcept>
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
}

namespace cre {



struct BitMatrix {
    uint32_t cap_row_bytes;
    uint32_t cap_col_bytes;
    uint32_t n_rows;
    uint32_t n_cols;
    uint8_t* data = nullptr;

    void reserve(uint32_t n_rows, uint32_t n_cols) {
        // Reserve in 64-bit chunks.
        uint32_t n_long_rows = (n_rows + 63) >> 8;
        uint32_t n_long_cols = (n_cols + 63) >> 8;
        uint32_t byte_width_rows = n_long_rows << 3;
        uint32_t byte_width_cols = n_long_cols << 3;
        if(cap_row_bytes < byte_width_rows || cap_col_bytes < byte_width_cols) {
            if(data != nullptr) free(data);
            data = (uint8_t*) malloc(byte_width_rows * byte_width_cols * sizeof(uint8_t));
            cap_row_bytes = byte_width_rows;
            cap_col_bytes = byte_width_cols;
        }
    }
    inline uint8_t set_byte(uint32_t byte_row_ind, uint32_t byte_col_ind, uint8_t value) {
        data[byte_row_ind * cap_col_bytes + byte_col_ind] = value;
    }

    inline uint8_t get_byte(uint32_t byte_row_ind, uint32_t byte_col_ind) const {
        return data[byte_row_ind * cap_col_bytes + byte_col_ind];
    }

    inline bool get(uint32_t row, uint32_t col) const {
        uint8_t byte = get_byte(row >> 3, col >> 3);
        return (byte >> (col & 0x7)) & 1;
    }

    inline void set(uint32_t row, uint32_t col, bool value) {
        uint8_t byte = get_byte(row >> 3, col >> 3);
        byte = (byte & ~(1 << (col & 0x7))) | (value << (col & 0x7));
        set_byte(row >> 3, col >> 3, byte);
    }

    BitMatrix(std::size_t n_rows, std::size_t n_cols):
        n_rows(n_rows), n_cols(n_cols), data(n_rows * n_cols, false)
    {};

    bool get(std::size_t row, std::size_t col) const {
        return data[row * n_cols + col];
};


constexpr static uint8_t CHANGE_KIND_DECLARE = 1; 
constexpr static uint8_t CHANGE_KIND_RETRACT = 2; 
constexpr static uint8_t CHANGE_KIND_MODIFY  = 3; 

struct ChangeEvent {
    int64_t f_id;
    int16_t mbr_ind; 
    uint16_t deref_kind;
    uint8_t change_kind;
    uint8_t pad[1]; 

    ChangeEvent(int64_t f_id, uint8_t change_kind=CHANGE_KIND_DECLARE) :
        f_id(f_id), mbr_ind(-1), deref_kind(DEREF_KIND_NULL),
        change_kind(change_kind)
    {};

    ChangeEvent(int64_t f_id, const DerefInfo& inf) :
        f_id(f_id), mbr_ind(-1), deref_kind(inf.deref_kind),
        change_kind(CHANGE_KIND_MODIFY)
    {};


    /* Comparators intended for binary map / hashmap lookups 
       change_kind not used here since it rarely matters for 
       triggering a change */
    inline bool operator<(const ChangeEvent& rec2) const {
        if(this->f_id != rec2.f_id) return this->f_id < rec2.f_id;
        if(this->mbr_ind !=  rec2.mbr_ind) return this->mbr_ind < rec2.mbr_ind;
        if(this->deref_kind !=  rec2.deref_kind) return this->deref_kind < rec2.deref_kind;        
    }

    inline bool operator==(const ChangeEvent& rec2) const {
        return (this->f_id == rec2.f_id &&
                this->mbr_ind ==  rec2.mbr_ind &&
                this->deref_kind ==  rec2.deref_kind);
    }
};

struct InputEntryState {
    // The input fact's f_id
    int64_t f_id;

    // For beta nodes the number of facts paired with this one. 
    int64_t true_count = 0;

    // The index of input fact in the associated output.
    int64_t output_ind = -1;

    // Some change to this entry has occured and it hasn't been checked yet.
    bool needs_check = true;

    // Indicates that the input fact is not removed
    bool source_valid = false;

    // Indicates that the input fact is not removed and its relevant 
    //  attributes have been successfully dereferenced.
    bool deref_is_valid = true;
    

    // // The input fact was inserted in this match cycle.
    // bool recently_inserted = false;

    // // The input fact was modified in this match cycle.
    // bool recently_modified = false;

    // // The input fact was removed or invalidated in this match cycle.
    // bool recently_invalid = false;

    

    // The input fact was matched in the previous match cycle 
    // bool true_was_nonzero = false;

    // The input fact has ever been a match. Used to keep track of 
    //  where holes should be kept in this input's corresponding output. 
    // bool true_ever_nonzero = false;
    bool pad[1];

    InputEntryState(int64_t f_id):
        f_id(f_id) 
    {};

    // void reset_temp_flags() {
    //     changed = false;
    //     recently_inserted = false;
    //     recently_modified = false;
    //     recently_invalid = false;
    // }
};



struct CORGI_IO {
    size_t _size;
    
    // Indicies changed in this match cycle.
    // std::vector<int64_t> changed_inds = {};
    // Indicies removed in this match cycle.
    // std::vector<int64_t> remove_inds = {};

    // The set of f_ids held by this input/output.
    std::vector<int64_t> match_f_ids = {};
    // The indicies in the input of associated output facts.
    std::vector<int64_t> match_inp_inds = {};

    // A mapping of idrecs to their indicies in the output.
    // std::map<uint64_t, int64_t> idrecs_to_inds = {};

    // A vector that keeps track indicies of holes in the output.  
    std::vector<int64_t> match_holes = {};
    // The size of the output including holes.  

    // int64_t width = 0;
    // A weak pointer to the node upstream to this one
    CORGI_Node* upstream_node = nullptr;

    // A vector of input states that observe this output.
    std::vector<InputState*> downstream_input_states = {};

    bool is_root = false;

    inline int64_t size() const {
        return match_f_ids.size();
    }

    void downstream_signal_add(size_t ind){
        for(InputState* input_state : downstream_input_states){
            InputEntryState& e_state = input_state->entry_states[ind];
            e_state.needs_check = true;
            input_state->changed_inds.push_back(ind);
        }
    }

    void downstream_signal_remove(size_t ind){
        for(InputState* input_state : downstream_input_states){
            InputEntryState& e_state = input_state->entry_states[ind];
            e_state.needs_check = false;
            e_state.source_valid = false;
            e_state.deref_is_valid = false;
            input_state->changed_inds.push_back(ind);
        }
    }
};

using HeadPtrTensor = Eigen::Tensor<Item*, 2, Eigen::RowMajor>;

//TODO: HeadsState
struct InputState {
    // A vector of idrecs that is filled directly in parse_change_events()
    //  on each match cycle to indicate a modify() relevant to this node.

    CORGI_Node* node;
    CORGI_IO* input;
    std::vector<Var*> head_var_ptrs;
    HeadRange head_range;
    // std::span<HeadInfo> head_infos;

    size_t _capacity; 
    size_t _size; 
    size_t n_heads; 
    std::vector<int64_t> modified_f_ids;
    
    HeadPtrTensor head_ptrs;
    std::vector<InputEntryState> entry_states;


    std::vector<int64_t> changed_inds = {};
    // std::vector<int64_t> c_buffer;
    // std::span<int64_t> changed_inds;
    // std::span<int64_t> unchanged_inds;
    // std::span<int64_t> removed_inds;



    inline size_t size(){return _size;}
    inline size_t capacity(){return _capacity;}

    InputState(CORGI_Node* node, size_t arg_ind){
        head_var_ptrs = {};

        auto kind = node->literal->kind;
        if(kind == LIT_KIND_EQ || kind == LIT_KIND_FUNC){
            Func* func = (Func*) node->literal->obj;
            head_range = func->head_ranges[arg_ind];
            for(size_t i=head_range.start; i < head_range.end; i++){
                const HeadInfo& hi = func->head_infos[i];
                head_var_ptrs.push_back(hi.var_ptr);
            }
        }else{
            throw std::runtime_error("NOT IMPLEMENTED");
        }
    }
    
    

    void signal_add(size_t ind){
        ensure_larger_than(ind + 1);
        InputEntryState& e_state = entry_states[ind];
        e_state.needs_check = true;
        e_state.source_valid = true;
        changed_inds.push_back(ind);
    }

    void signal_remove(size_t ind){
        InputEntryState& e_state = entry_states[ind];
        e_state.needs_check = false;
        e_state.source_valid = false;
        e_state.deref_is_valid = false;
        changed_inds.push_back(ind);
    }

    void signal_modify(size_t ind){
        InputEntryState& e_state = entry_states[ind];
        e_state.needs_check = true;
        changed_inds.push_back(ind);
    }

    void ensure_larger_than(size_t new_size){
        if(new_size > size()){
            size_t new_cap = std::max(new_size, _capacity + std::min(64,_capacity));
            head_ptrs.conservativeResize(new_cap, n_heads);
            entry_states.reserve(new_cap);
        }
    }

    
    bool validate_head_or_retract(int64_t f_id, size_t change_ind){

        bool is_valid = true;

        // for(Var* head_var : head_var_ptrs){
        size_t v = 0;
        for(size_t i=head_range.start; i < head_range.end; i++){
            Var* head_var = head_var_ptrs[i];
            Item* head_ptr = resolve_head_ptr(f_id, head_var, change_ind);
            if(head_ptr == nullptr){
                is_valid = false;
                break;
            }
            head_ptrs(change_ind, i) = head_ptr;
        }
        return is_valid;


    }

    Item* resolve_head_ptr(int64_t f_id, Var* var, int64_t ind=-1){
        const std::vector<ref<Fact>>& facts = node->fact_set->facts;
        size_t n_derefs = var->length;
        DerefInfo* deref_infos = var->deref_infos;
        if(n_derefs > 0){
            Fact* inst_ptr = (Fact*) facts[f_id].get();
            if(n_derefs > 1){
                for(int i=0; i < n_derefs-1; i++){
                    if(inst_ptr == nullptr) return nullptr;

                    const DerefInfo& deref_info = deref_infos[i];
                    Member* mbr_ptr = deref_once(inst_ptr, deref_info);
                    
                    // Insert a ChangeEventord into change_dep_map so we can 
                    //  invalidate the relevant input entry if the dereference
                    //  chain is invalidated by a retraction or modify
                    if(ind != -1){
                        ChangeDependsMap& change_dep_map = node->graph->change_dep_map; 
                        ChangeEvent cr = ChangeEvent(f_id, deref_info);
                        auto it = change_dep_map.find(cr);
                        bool inserted = false;
                        if(it == change_dep_map.end()){
                            std::tie(it, inserted) = change_dep_map.insert(
                                {cr, std::vector<ChangeDep>()}
                            );                        
                        }
                        it->second.push_back(ChangeDep(this, ind));
                    }
                    inst_ptr = (Fact*) mbr_ptr->get_ptr();
                }
                if(inst_ptr != nullptr){
                    const DerefInfo& deref = deref_infos[-1];
                    Member* mbr_ptr = deref_once(inst_ptr, deref);
                    return (Item*) mbr_ptr;
                }else{
                    return nullptr;
                }
            }
        }
    }
    
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

    void _resolve_head(size_t change_ind, bool is_modify){
        int64_t f_id = input->match_f_ids[change_ind];
        InputEntryState& e_state = entry_states[change_ind]; 

        bool was_valid = e_state.is_valid;
        
        bool is_valid = validate_head_or_retract(f_id, change_ind);
        bool recently_inserted = (!was_valid & is_valid);
        bool recently_modified = (is_modify & is_valid);

        // Assign changes to the input_state struct.
        e_state.f_id = f_id;
        if(was_valid && !is_valid){
            e_state.recently_invalid = true;
        }
        e_state.recently_inserted = recently_inserted;
        e_state.recently_modified = recently_modified;
        e_state.is_valid = is_valid;

    }
        


    void update(){

        // Ensure various buffers are large enough
        

        // for(auto& istate : entry_states){
        //     istate.reset_temp_flags();
        // }

        // Update input_states with any upstream removals
        size_t n_rem = 0;
        for(int64_t ind : input->remove_inds){
            InputEntryState& e_state = entry_states[ind];
            if(e_state.is_valid){
                ++n_rem;
                e_state.recently_invalid = true;
            }
            e_state.is_valid = false;
            e_state.changed = true;
        }

        for(int64_t ind : input->changed_inds){
            InputEntryState& e_state = entry_states[ind];
            e_state.changed = true;
            _resolve_head(ind, false);
        }
        
        // size_t c = 0;
        // Add to change_inds any modifies specifically routed to this node. 
        for(int64_t ind : input->modify_events){
            _resolve_head(ind, true);
        }


        
    }
};

struct DependsComparator {
    /** Comparator over ChangeEvents.
    */
    bool operator()(const ChangeEvent& rec1, const ChangeEvent& rec2) const {
        if(rec1.f_id !=  rec2.f_id) return rec1.f_id < rec2.f_id;
        if(rec1.mbr_ind !=  rec2.mbr_ind) return rec1.mbr_ind < rec2.mbr_ind;
        if(rec1.mbr_ind !=  rec2.mbr_ind) return rec1.mbr_ind < rec2.mbr_ind;
        
    }
};

struct ChangeDep {
    // An input state in a node that depends on this change event.
    InputState* input_state;
    // The index of the input fact in the input state.
    size_t ind;

    ChangeDep(InputState* input_state, size_t ind):
        input_state(input_state), ind(ind)
    {};
};


                 
struct CORGI_Node {
    // Weak pointer to graph this node belongs to
    CORGI_Graph* graph;

    // Weak pointer to the working memory for this graph
    FactSet* fact_set;

    // The Literal associated with this node
    ref<Literal> literal;

    // The Op for the node's literal
    // ref<Func> func;

    
    
    // The number of Vars (1 for alpha or 2 for beta)
    size_t n_vars;

    // The var_inds for the vars handled by this node 
    VarInds var_inds;
    // t_ids
    std::vector<CORGI_IO> inputs;
    std::vector<CORGI_IO> outputs;
    std::vector<InputState> input_states;
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


using ChangeDependsMap = std::map<ChangeEvent, std::vector<ChangeDep>>;

struct CORGI_Graph {
    // The change_head of the working memory at the last graph update.
    size_t change_head = 0;

    // The working memory memset.
    FactSet* fact_set = nullptr;

    // All graph nodes organized by [[...alphas],[...betas],[...etc]]
    std::vector<std::vector<CORGI_Node*>> nodes_by_nargs = {};

    // The number of nodes in the graph
    size_t n_nodes = 0;

    // List of root nodes (i.e. the nodes that holds all
    //  match candidates for a fact_type before filtering).
    std::vector<CORGI_Node*> root_nodes = {};

    // Maps types to the root node outputs associated with facts of that type.
    std::map<CRE_Type*, CORGI_IO*> root_map = {};

    // ???
    ChangeDependsMap change_dep_map = {};

    // Maps (t_id, 0, a_id) idrec patterns to (node,arg_ind) that should be 
    //  rechecked based on that pattern.
    // std::map<uint64_t, std::vector<std::pair<CORGI_Node*, size_t>>> global_modify_map;

    // The sum of weights of all literals in the graph
    float total_structure_weight = 0.0f;
    float total_match_weight = 0.0f;

    void parse_change_events(){
        for(size_t i=change_head; i < fact_set->change_queue.size(); i++){
            ChangeEvent& change_event = &fact_set->change_queue[i];
            Fact* fact = fact_set->get(change_event.f_id);
            
            // Modifies route directly to their dependencies 
            auto change_dep_it = change_dep_map.find(change_event);
            std::vector<ChangeDep>& change_deps = change_dep_it->second;
            for(ChangeDep& change_dep : change_deps){
                InputState* input_state = change_dep.input_state;
                input_state->changed_inds.push_back(change_dep.ind);
            }

            // DECLARE / RETRACT route to their root inputs and propogate
            //  down the CORGI graph.
            if(change_event.change_kind == CHANGE_KIND_DECLARE || 
               change_event.change_kind == CHANGE_KIND_RETRACT){
                auto root_it = root_map.find(fact->type);
                if(root_it == root_map.end()) continue;
                CORGI_IO* root_io = root_it->second;

                root_io->changed_inds.push_back(change_event.f_id);
                if(change_event.change_kind == CHANGE_KIND_RETRACT){
                    root_io->match_f_ids.push_back(change_event.f_id);
                }else{
                    root_io->remove_inds.push_back(change_event.f_id);
                }
                
            }
        }
    }
};

// Node Update Phases
// 1) Process the input changes in each InputState
// 2) Resolve the head pointers of any 
// 3) Revaluate the relation along all changes or pairs of changes 
// 4) Update the output, and its changes 



} // namespace cre