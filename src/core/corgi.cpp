#include "../include/corgi.h"
#include <cstdlib>

namespace cre {

// BitMatrix implementations
void BitMatrix::reserve(uint32_t n_rows, uint32_t n_cols) {
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

void BitMatrix::set_byte(uint32_t byte_row_ind, uint32_t byte_col_ind, uint8_t value) {
    data[byte_row_ind * cap_col_bytes + byte_col_ind] = value;
}

uint8_t BitMatrix::get_byte(uint32_t byte_row_ind, uint32_t byte_col_ind) const {
    return data[byte_row_ind * cap_col_bytes + byte_col_ind];
}

bool BitMatrix::get(uint32_t row, uint32_t col) const {
    uint8_t byte = get_byte(row >> 3, col >> 3);
    return (byte >> (col & 0x7)) & 1;
}

void BitMatrix::set(uint32_t row, uint32_t col, bool value) {
    uint8_t byte = get_byte(row >> 3, col >> 3);
    byte = (byte & ~(1 << (col & 0x7))) | (value << (col & 0x7));
    set_byte(row >> 3, col >> 3, byte);
}

BitMatrix::BitMatrix(std::size_t n_rows, std::size_t n_cols):
    n_rows(n_rows), n_cols(n_cols)
{
    reserve(n_rows, n_cols);
}

bool BitMatrix::get(std::size_t row, std::size_t col) const {
    return data[row * n_cols + col];
}

// InputEntryState implementations
InputEntryState::InputEntryState(int64_t f_id):
    f_id(f_id) 
{}

// CORGI_IO implementations
CORGI_IO::CORGI_IO(CORGI_Node* upstream_node, size_t arg_ind) :
    upstream_node(upstream_node), arg_ind(arg_ind) 
{}

int64_t CORGI_IO::size() const {
    return _size;
}

int64_t CORGI_IO::capacity() const {
    return match_f_ids.size();
}

void CORGI_IO::downstream_signal_add(size_t ind, int64_t f_id){
    
    for(InputState* input_state : downstream_input_states){
        // cout << "DOWNSTREAM SIGNAL ADD: " << ind << " " << f_id << "->" << uint64_t(input_state) << endl;
        input_state->signal_add(ind, f_id);
    }
}

void CORGI_IO::downstream_signal_remove(size_t ind){
    for(InputState* input_state : downstream_input_states){
        input_state->signal_remove(ind);
    }
}

int64_t CORGI_IO::add(int64_t f_id, int64_t input_ind){
    // cout << fmt::format("ADD: f_id={}, input_ind={}", f_id, input_ind) << endl;
    if(match_holes.size() > 0){
        int64_t ind = match_holes.back();
        match_holes.pop_back();
        match_f_ids[ind] = f_id;
        input_inds[ind] = input_ind;
        return ind;
    }else{
        match_f_ids.push_back(f_id);
        input_inds.push_back(input_ind);
        return match_f_ids.size() - 1;
    }
}

void CORGI_IO::add_at(size_t ind, int64_t f_id, int64_t input_ind){
    if(ind >= capacity()){
        size_t new_cap = std::max(ind + 1, _size + std::min(64ul,_size));
        match_f_ids.resize(new_cap, -1);
    }
    match_f_ids[ind] = f_id;
    input_inds[ind] = input_ind;
}

void CORGI_IO::remove_at(size_t ind){
    match_f_ids[ind] = -1;
    match_holes.push_back(ind);
}


std::string CORGI_IO::to_string() const{
    std::stringstream ss;
    if(upstream_node != nullptr){
        std::string var_str = arg_ind >= 0 ? upstream_node->input_states[arg_ind].head_var_ptrs[0]->base->to_string() : "??";
        cout << fmt::format("Output ({}) of node: {}" , var_str, upstream_node->literal->to_string()) << endl;
    }else{
        cout << "Root IO:" << endl;
    }
    for(size_t i=0; i < match_f_ids.size(); i++){
        int64_t f_id = match_f_ids[i];
        ss << f_id << " : ";
        FactSet* fact_set = upstream_node != nullptr ? upstream_node->graph->fact_set : nullptr;
        if(f_id != -1 && fact_set != nullptr){
            ref<Fact> fact = fact_set->get(f_id);
            ss << fact->to_string() << " ";
        }else{
            ss << "null ";
        }
        ss << endl;
    }
    return ss.str();
}
HeadValueBuffer::HeadValueBuffer() {};

HeadValueBuffer::HeadValueBuffer(Func* func, size_t arg_ind) :
    byte_width(0),
    n_heads(func->head_ranges[arg_ind].end-func->head_ranges[arg_ind].start)
{
    head_offsets.reserve(n_heads);
    for(size_t i=func->head_ranges[arg_ind].start; i < func->head_ranges[arg_ind].end; i++){
        const HeadInfo& hi = func->head_infos[i];
        head_offsets.push_back(byte_width);
        byte_width += hi.head_type->byte_width;
    }
    resize(64);
}

void HeadValueBuffer::resize(size_t n_entries){
    if(n_entries > capacity){
        size_t new_capacity = std::max(n_entries, capacity + std::min(64ul,capacity));
        void* new_data = (void*) malloc(new_capacity * byte_width);
        if(data != nullptr){
            memcpy(new_data, data, capacity * byte_width);
            free(data);
        }
        data = new_data;
        capacity = new_capacity;
        
    }
}

// ChangeDep implementations
ChangeDep::ChangeDep(InputState* input_state, size_t ind, bool is_deref_support):
    input_state(input_state), ind(ind), is_deref_support(is_deref_support)
{}

// InputState implementations
size_t InputState::size(){return _size;}

size_t InputState::capacity(){return _capacity;}

void InputState::ensure_larger_than(size_t new_size, size_t new_capacity){
    if(new_size > size()){
        new_capacity  = new_capacity == -1 ? new_size : new_capacity;
        size_t new_cap = std::max(new_capacity, _capacity + std::min(64ul,_capacity));
        // head_ptrs.conservativeResize(new_cap, n_heads);
        head_value_buffer.resize(new_cap);
        entry_states.resize(new_cap, InputEntryState(-1));
        _capacity = new_cap;
        _size = new_size;
    }
}

InputState::InputState(CORGI_Node* node, size_t arg_ind) :
    node(node), input(node->inputs[arg_ind]), 
    _capacity(input->capacity()), _size(input->size())
{
    head_var_ptrs = {};

    auto kind = node->literal->kind;
    size_t arg_offset = 0;
    if(kind == LIT_KIND_EQ || kind == LIT_KIND_FUNC){
        Func* func = (Func*) node->literal->obj.get();
        head_range = func->head_ranges[arg_ind];
        n_heads = head_range.end - head_range.start;
        for(size_t i=head_range.start; i < head_range.end; i++){
            const HeadInfo& hi = func->head_infos[i];
            head_var_ptrs.push_back(hi.var_ptr);
            arg_offset += hi.base_type->byte_width;
        }
        head_value_buffer = HeadValueBuffer(func, arg_ind);
    }else{
        throw std::runtime_error("NOT IMPLEMENTED");
    }
    // cout << "head_var_ptrs: " << head_var_ptrs.size() << endl;
    ensure_larger_than(input->size(), input->capacity());
}

void InputState::signal_add(size_t ind, int64_t f_id){
    ensure_larger_than(ind + 1);
    InputEntryState& e_state = entry_states[ind];
    e_state.source_valid = true;
    e_state.needs_head_check = true;
    e_state.needs_update_relation = true;
    e_state.f_id = f_id;
    changed_inds.push_back(ind);
    // cout << fmt::format("{}: SIGNAL ADD: ind={}, f_id={}, {} @ {}", head_var_ptrs[0]->base->to_string(), ind, f_id, changed_inds, uint64_t(this)) << endl;
}

void InputState::signal_remove(size_t ind){
    InputEntryState& e_state = entry_states[ind];
    e_state.source_valid = false;
    e_state.needs_head_check = false;
    e_state.head_is_valid = false;
    e_state.needs_update_relation = true;
    changed_inds.push_back(ind);
}

void InputState::signal_modify(size_t ind, bool is_deref_support){
    InputEntryState& e_state = entry_states[ind];
    if(is_deref_support){
        e_state.needs_head_check = true;
    }
    e_state.needs_update_relation = true;
    changed_inds.push_back(ind);
}



bool InputState::validate_head_or_retract(int64_t f_id, size_t change_ind){

    bool is_valid = true;

    // for(Var* head_var : head_var_ptrs){
    void* entry_data_ptr = head_value_buffer.get_entry_data_ptr(change_ind);
    size_t j = 0;
    for(size_t i=0; i < n_heads; i++){
        Var* head_var = head_var_ptrs[i];
        Item* head_ptr = resolve_head_ptr(f_id, head_var, change_ind);
        if(head_ptr == nullptr){
            is_valid = false;
            break;
        }
        
        void* dest = (uint8_t*) entry_data_ptr + head_value_buffer.head_offsets[j];
        copy_convert_arg(dest, *head_ptr, head_var->head_type);
        // head_ptrs(change_ind, i) = head_ptr;
        j++;
    }
    return is_valid;


}

void InputState::insert_change_dep(int64_t f_id, 
                      const DerefInfo& deref_info,
                      int64_t ind, bool is_deref_support){
    ChangeDependsMap& change_dep_map = node->graph->change_dep_map; 
    ChangeEvent cr = ChangeEvent(f_id, deref_info);
    auto it = change_dep_map.find(cr);
    bool inserted = false;
    if(it == change_dep_map.end()){
        std::tie(it, inserted) = change_dep_map.insert({cr, std::vector<ChangeDep>()});
    }
    it->second.push_back(ChangeDep(this, ind, is_deref_support));
}

Item* InputState::resolve_head_ptr(int64_t f_id, Var* var, int64_t ind){
    const std::vector<ref<Fact>>& facts = node->graph->fact_set->facts;
    size_t n_derefs = var->length;
    DerefInfo* deref_infos = var->deref_infos;
    Fact* inst_ptr = (Fact*) facts[f_id].get();
    if(n_derefs > 0){
        if(n_derefs > 1){
            for(int i=0; i < n_derefs-1; i++){
                if(inst_ptr == nullptr) return nullptr;

                const DerefInfo& deref_info = deref_infos[i];
                Member* mbr_ptr = deref_once(inst_ptr, deref_info);
                
                // Insert a ChangeEventord into change_dep_map so we can 
                //  invalidate the relevant input entry if the dereference
                //  chain is invalidated by a retraction or modify
                if(ind != -1){
                    insert_change_dep(f_id, deref_info, ind, true);
                }
                inst_ptr = (Fact*) mbr_ptr->get_ptr();
            }
        }
        if(inst_ptr != nullptr){
            const DerefInfo& deref_info = deref_infos[n_derefs-1];
            Member* mbr_ptr = deref_once(inst_ptr, deref_info);
            insert_change_dep(f_id, deref_info, ind, false);
            return (Item*) mbr_ptr;
        }else{
            return nullptr;
        }
    }
    return nullptr;
}

void InputState::update(){
    // cout << head_var_ptrs[0]->base->to_string() << ": changed_inds: " << fmt::format("{} @ {}", changed_inds, uint64_t(this)) << endl;
    for(int64_t change_ind : changed_inds){
        InputEntryState& e_state = entry_states[change_ind];
        bool head_was_valid = e_state.head_is_valid;

        // Follow the var's deref chain to validate address of the terminating 
        //  head field in some fact.
        bool head_is_valid = true;
        if(e_state.needs_head_check){
            head_is_valid = validate_head_or_retract(e_state.f_id, change_ind);
        }
        e_state.head_is_valid = head_is_valid;
        
        // Always update unless head was invalid and is still invalid
        e_state.needs_update_relation = !(!head_was_valid & !head_is_valid);
    }
    

    // Ensure various buffers are large enough
    

    // for(auto& istate : entry_states){
    //     istate.reset_temp_flags();
    // }

    // Update input_states with any upstream removals
    // size_t n_rem = 0;
    // for(int64_t ind : input->remove_inds){
    //     InputEntryState& e_state = entry_states[ind];
    //     if(e_state.is_valid){
    //         ++n_rem;
    //         e_state.recently_invalid = true;
    //     }
    //     e_state.is_valid = false;
    //     e_state.changed = true;
    // }

    
    
    // size_t c = 0;
    // Add to change_inds any modifies specifically routed to this node. 
    // for(int64_t ind : input->modify_events){
    //     _resolve_head(ind, true);
    // }


    
}

// CORGI_Node implementations
CORGI_Node::CORGI_Node(CORGI_Graph* graph, Literal* literal, const std::vector<CORGI_IO*>& inputs) :
    graph(graph), literal(literal), inputs(inputs) {
    // n_vars = literal->var_inds.size();
    // var_inds = literal->var_inds;

    // NOTE: reserve() is necessary here so pointer held by downstream 
    //   nodes is not invalidated as input_states is resized.
    input_states.reserve(inputs.size()); 
    outputs.reserve(inputs.size());
    for(size_t i=0; i < inputs.size(); i++){
        input_states.emplace_back(InputState(this, i));
        inputs[i]->downstream_input_states.push_back(&input_states[i]);
        outputs.emplace_back(std::make_unique<CORGI_IO>(this, i));
    }
}

void CORGI_Node::update_alpha_matches_func(){
    InputState& input_state = input_states[0];
    Func* func = (Func*) literal->obj.get();
    // size_t n_heads = func->head_infos.size();
    for(auto& change_ind : input_state.changed_inds){
        InputEntryState& e_state = input_state.entry_states[change_ind];
        
        // void** row_ptr = (void**) input_state.head_ptrs.row(change_ind).data();
        bool is_match = (eval_func_relation(func, change_ind) == CFSTATUS_TRUTHY) ^ literal->negated;
        e_state.true_count = int64_t(is_match);
    }
}


void CORGI_Node::update_beta_matches_func(){

    // Resize the truth table to the size of the inputs
    size_t n_rows = truth_table.rows();
    size_t n_cols = truth_table.cols();
    truth_table.conservativeResize(
        input_states[0].size(), input_states[1].size());
    for(size_t i=n_rows; i < input_states[0].size(); i++){
        for(size_t j=n_cols; j < input_states[1].size(); j++){
            truth_table(i, j) = false;
        }
    }

    Func* func = (Func*) literal->obj.get();

    auto update_truth_table_cell = ([&](
        bool is_match, size_t i, size_t j,
        InputEntryState& es_i, InputEntryState& es_j
    ){    
        bool was_match = truth_table(i, j);
        truth_table(i, j) = is_match;
        int64_t count_diff = int64_t(is_match) - int64_t(was_match);
        es_i.true_count += count_diff;
        es_j.true_count += count_diff;
    });

    // for(size_t arg_ind = 0; arg_ind < 2; arg_ind++){
    // size_t other_ind = arg_ind == 0 ? 1 : 0;
    InputState& inp_state0 = input_states[0];
    InputState& inp_state1 = input_states[1];
    if(upstream_same_parents){
        auto& u_tt = inputs[0]->upstream_node->truth_table;
        auto& u_input_inds0 = inputs[0]->input_inds;
        auto& u_input_inds1 = inputs[1]->input_inds;

        auto upstream_true = ([&](size_t i, size_t j) -> bool {
            return u_tt(u_input_inds0[i], u_input_inds1[j]);
        });

        // Update the whole row/column
        for(size_t k=0; k < inp_state0.changed_inds.size(); k++){
            int64_t i = inp_state0.changed_inds[k];
            InputEntryState& es_i = inp_state0.entry_states[i];
            for(size_t j=0; j < inp_state1.size(); j++){
                InputEntryState& es_j = inp_state1.entry_states[j];

                // If the upstream truth table is false, then this cell is also false
                if(not upstream_true(i, j)){
                    update_truth_table_cell(false, i, j, es_i, es_j);    
                    continue;
                }
                bool is_match = false;
                if(es_j.head_is_valid){
                    is_match = (eval_func_relation(func, i, j) == CFSTATUS_TRUTHY) ^ literal->negated;
                }
                update_truth_table_cell(is_match, i, j, es_i, es_j);
            }
        }
        // Check just the unchanged parts, so to avoid repeat checks 
        for(size_t k=0; k < inp_state1.changed_inds.size(); k++){
            int64_t j = inp_state1.changed_inds[k];
            InputEntryState& es_j = inp_state1.entry_states[j];
            for(size_t i=0; i < inp_state0.size(); i++){
                InputEntryState& es_i = inp_state1.entry_states[i];

                // If the upstream truth table is false, then this cell is also false
                if(not upstream_true(i, j)){
                    update_truth_table_cell(false, i, j, es_i, es_j);    
                    continue;          
                }
                bool is_match = false;
                if(es_j.head_is_valid){
                    is_match = (eval_func_relation(func, i, j) == CFSTATUS_TRUTHY) ^ literal->negated;
                }
                update_truth_table_cell(is_match, i, j, es_i, es_j);
            }
        }
    }else{
        // Update the whole row/column
        for(size_t k=0; k < inp_state0.changed_inds.size(); k++){
            int64_t i = inp_state0.changed_inds[k];
            InputEntryState& es_i = inp_state0.entry_states[i];
            for(size_t j=0; j < inp_state1.size(); j++){
                InputEntryState& es_j = inp_state1.entry_states[j];
                bool is_match = false;
                if(es_j.head_is_valid){
                    is_match = (eval_func_relation(func, i, j) == CFSTATUS_TRUTHY) ^ literal->negated;
                }
                update_truth_table_cell(is_match, i, j, es_i, es_j);
            }
        }
        // Check just the unchanged parts, so to avoid repeat checks 
        // Update the whole row/column
        for(size_t k=0; k < inp_state1.changed_inds.size(); k++){
            int64_t j = inp_state1.changed_inds[k];
            InputEntryState& es_j = inp_state1.entry_states[j];
            for(size_t i=0; i < inp_state0.size(); i++){
                InputEntryState& es_i = inp_state0.entry_states[i];
                // Skip entries updated along dim 0 because they were already updated
                if(!es_i.needs_update_relation) continue;          

                bool is_match = false;
                if(es_j.head_is_valid){
                    is_match = (eval_func_relation(func, i, j) == CFSTATUS_TRUTHY) ^ literal->negated;
                }
                update_truth_table_cell(is_match, i, j, es_i, es_j);
            }
        }
    }
}

void CORGI_Node::update_output_changes(){

    // cout << truth_table << endl;
    // cout << "--------" << endl;
    for(size_t i=0; i < input_states.size(); i++){
        InputState& input_state = input_states[i];
        CORGI_IO* output = outputs[i].get();
        for(size_t k=0; k < input_state.entry_states.size(); k++){
            InputEntryState& e_state = input_state.entry_states[k];
            
            bool true_is_nonzero = (e_state.true_count > 0);
            e_state.true_ever_nonzero |= true_is_nonzero;
            
            // Skip other checks if this fact has never been a match.
            if(!e_state.true_ever_nonzero) continue;

            // If the fact is not present in the output and is now a match,
            //  add it to the output and signal the downstream nodes.
            if(!e_state.present_in_output && true_is_nonzero){
                e_state.present_in_output = true;
                auto output_ind = output->add(e_state.f_id);
                // cout << fmt::format("ADD ind={}, f_id={}", output_ind, e_state.f_id) << endl;
                e_state.output_ind = output_ind;
                output->downstream_signal_add(output_ind, e_state.f_id);
            
            // If the fact is present in the output and is now not a match,
            //  add it to the output and signal the downstream nodes.
            }else if(e_state.present_in_output && !true_is_nonzero){
                e_state.present_in_output = false;
                // cout << fmt::format("REMOVE ind={}, f_id={}", e_state.output_ind, e_state.f_id) << endl;
                outputs[k]->remove_at(e_state.output_ind);
                output->downstream_signal_remove(e_state.output_ind);
                e_state.output_ind = -1;
            }
        }

        cout << output->to_string() << endl;
        // Clear the changed indicies for this input state to indicate
        //  that they have been processed (during input updates and match checking)
        input_state.changed_inds.clear();
    }

}

void CORGI_Node::update(){
    for(InputState& input_state : input_states){
        input_state.update();
    }

    if(literal->var_inds.size() == 1){
        if(literal->kind == LIT_KIND_FUNC){
            update_alpha_matches_func();
        }
    }else{
        if(literal->kind == LIT_KIND_FUNC){
            update_beta_matches_func();
        }
    }

    // cout << "NOW!!!" << endl;

    update_output_changes();

    for(InputState& input_state : input_states){
        input_state.changed_inds.clear();
    }
}

// VarFrontier implementations
VarFrontier::VarFrontier(CORGI_IO* output) : output(output) {}

// CORGI_Graph implementations
void CORGI_Graph::_ensure_root_inputs(Logic* logic){
    auto ensure_root_io = ([&](int32_t type_index){
        if(root_inputs[type_index] == nullptr){
            root_inputs[type_index] = std::move(std::make_unique<CORGI_IO>());
        }
    });
    for(size_t i=0; i < logic->vars.size(); i++){
        Var* var = logic->vars[i];
        CRE_Type* type = var->base_type;
        ensure_root_io(type->type_index);
        for(CRE_Type* subtype : type->sub_types){
            ensure_root_io(subtype->type_index);
        }
    }
}

// Function to get the indices that would sort an Eigen vector
template <typename T>
std::vector<size_t> argsort_eigen(const T& vec) {
    // Initialize a vector of indices from 0 to n-1
    std::vector<size_t> indices(vec.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on the values in the original vector
    std::sort(indices.begin(), indices.end(),
        [&](size_t i, size_t j) {
            return vec(i) < vec(j);
        });

    return indices;
}

std::vector<size_t> CORGI_Graph::_get_degree_order(Logic* logic){
    // Order vars by decreasing beta degree --- the number of other 
    // vars that they share beta literals with. Implements heuristic
    // that the most beta-constrained nodes are matched first. 
    auto has_pairs = Eigen::Matrix<bool, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>(
        logic->vars.size(), logic->vars.size()
    );
    for(Item& item : logic->items){
        if(item.get_t_id() == T_ID_LITERAL){
            Literal* lit = item._as<Literal*>();
            VarInds& var_inds = lit->var_inds;
            for(size_t i=0; i < var_inds.size(); i++){
                for(size_t j=i+1; j < var_inds.size(); j++){
                    has_pairs(var_inds[i], var_inds[j]) = true;
                    has_pairs(var_inds[j], var_inds[i]) = true;
                }
            }
        }
    }
    auto degree = has_pairs.colwise().sum();
    auto degree_order = argsort_eigen(degree);
    return degree_order;
}

void CORGI_Graph::_add_literal(Literal* lit, std::vector<VarFrontier>& frontiers){
    // Let the inputs to the node be the outputs of the frontiers
    //  of the literals' vars.
    std::vector<CORGI_IO*> inputs = {};
    for(size_t i=0; i < lit->var_inds.size(); i++){
        VarFrontier& frontier = frontiers[lit->var_inds[i]];
        inputs.push_back(frontier.output);
    }

    if(lit->var_inds.size() >= nodes_by_nargs.size()){
        nodes_by_nargs.resize(lit->var_inds.size()+1);
    }

    // Make the new node and add it to the graph.
    nodes.emplace_back(std::make_unique<CORGI_Node>(this, lit, inputs));
    CORGI_Node* node = nodes[nodes.size() - 1].get();//
        
    nodes_by_nargs[lit->var_inds.size()].push_back(node);

    // Update the frontiers to point to the outputs of the new node.
    for(size_t i=0; i < lit->var_inds.size(); i++){
        VarFrontier& frontier = frontiers[lit->var_inds[i]];
        frontier.output = node->outputs[i].get();
        frontier.upstream_depends.push_back(lit);
    }
    
}

void CORGI_Graph::_add_logic(Logic* logic, std::vector<VarFrontier>& frontiers){
    // auto degree_order = _get_degree_order(logic);
    if(logic->kind == CONDS_KIND_AND){
        // Add a node for each literal
        for(auto item : logic->items){
            if(item.get_t_id() == T_ID_LITERAL){
                _add_literal(item._as<Literal*>(), frontiers);
            }else if(item.get_t_id() == T_ID_LOGIC){
                _add_logic(item._as<Logic*>(), frontiers);
            }
        }
    }else if(logic->kind == CONDS_KIND_OR){
        // For ORs we copy the frontier so it is independent for each branch.
        for(auto item : logic->items){
            std::vector<VarFrontier> frontiers_copy = frontiers;
            if(item.get_t_id() == T_ID_LITERAL){
                _add_literal(item._as<Literal*>(), frontiers_copy);
            }else if(item.get_t_id() == T_ID_LOGIC){
                _add_logic(item._as<Logic*>(), frontiers_copy);
            }
        }
    }   

}

void CORGI_Graph::add_logic(Logic* logic){
    std::vector<VarFrontier> frontiers = {};
    frontiers.resize(logic->vars.size(), nullptr);
    for(size_t i=0; i < logic->vars.size(); i++){
        CRE_Type* var_type = logic->vars[i]->base_type;
        frontiers[i] = VarFrontier(get_root_io(var_type));
    }
    _add_logic(logic, frontiers);
    // TODO: Do I need to build var_end_join_ptrs?
}

CORGI_IO* CORGI_Graph::get_root_io(CRE_Type* type){
    int32_t type_index = type->type_index;
    if(type_index >= root_inputs.size()){
        root_inputs.resize(type_index + 1);
    }
    if(root_inputs[type_index] == nullptr){
        root_inputs[type_index] = std::make_unique<CORGI_IO>(nullptr);
    }
    return root_inputs[type_index].get();
}

void CORGI_Graph::parse_change_events(){
    for(size_t i=change_head; i < fact_set->change_queue.size(); i++){
        ChangeEvent& change_event = fact_set->change_queue[i];
        Fact* fact = fact_set->get(change_event.f_id);
        
        // Modifies route directly to their dependencies 
        auto change_dep_it = change_dep_map.find(change_event);
        if(change_dep_it != change_dep_map.end()){
            std::vector<ChangeDep>& change_deps = change_dep_it->second;
            for(ChangeDep& change_dep : change_deps){
                InputState* input_state = change_dep.input_state;
                input_state->signal_modify(change_dep.ind, change_dep.is_deref_support);
            }
        }

        // DECLARE / RETRACT route to their root inputs and propogate
        //  down the CORGI graph.
        if(change_event.change_kind == CHANGE_KIND_DECLARE || 
           change_event.change_kind == CHANGE_KIND_RETRACT){

            auto add_to_root_io = ([&](CORGI_IO* root_io){
                // Skip if the root io is nullptr.
                if(root_io == nullptr) return;
                if(change_event.change_kind == CHANGE_KIND_DECLARE){
                    size_t ind = root_io->add(change_event.f_id);
                    root_io->downstream_signal_add(ind, change_event.f_id);
                }else{
                    root_io->remove_at(change_event.f_id);
                    root_io->downstream_signal_remove(change_event.f_id);
                }
            });

            CORGI_IO* root_io = get_root_io(fact->type);
            add_to_root_io(root_io);
            for(CRE_Type* subtype : fact->type->sub_types){
                root_io = get_root_io(subtype);
                add_to_root_io(root_io);
            }
        }
    }
}

void CORGI_Graph::update(){
    
    parse_change_events();
    for(auto& node : nodes){
        // cout << fmt::format(" -- UPDATE NODE : {} --", node->literal->to_string()) << endl;
        node->update();
    }
}

} // namespace cre

