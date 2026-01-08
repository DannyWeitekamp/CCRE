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

uint8_t BitMatrix::set_byte(uint32_t byte_row_ind, uint32_t byte_col_ind, uint8_t value) {
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
CORGI_IO::CORGI_IO(CORGI_Node* upstream_node) :
    upstream_node(upstream_node) 
{}

int64_t CORGI_IO::size() const {
    return _size;
}

int64_t CORGI_IO::capacity() const {
    return match_f_ids.size();
}

void CORGI_IO::downstream_signal_add(size_t ind){
    for(InputState* input_state : downstream_input_states){
        input_state->signal_add(ind);
    }
}

void CORGI_IO::downstream_signal_remove(size_t ind){
    for(InputState* input_state : downstream_input_states){
        input_state->signal_remove(ind);
    }
}

int64_t CORGI_IO::add(int64_t f_id){
    if(match_holes.size() > 0){
        int64_t ind = match_holes.back();
        match_holes.pop_back();
        match_f_ids[ind] = f_id;
        return ind;
    }else{
        match_f_ids.push_back(f_id);
        return match_f_ids.size() - 1;
    }
}

void CORGI_IO::add_at(size_t ind, int64_t f_id){
    if(ind >= capacity()){
        size_t new_cap = std::max(ind + 1, _size + std::min(64,_size));
        match_f_ids.resize(new_cap, -1);
    }
    match_f_ids[ind] = f_id;
}

void CORGI_IO::remove_at(size_t ind){
    match_f_ids[ind] = -1;
    match_holes.push_back(ind);
}

// ChangeDep implementations
ChangeDep::ChangeDep(InputState* input_state, size_t ind, bool is_deref_support):
    input_state(input_state), ind(ind), is_deref_support(is_deref_support)
{}

// InputState implementations
size_t InputState::size(){return _size;}

size_t InputState::capacity(){return _capacity;}

InputState::InputState(CORGI_Node* node, size_t arg_ind){
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

void InputState::signal_add(size_t ind){
    ensure_larger_than(ind + 1);
    InputEntryState& e_state = entry_states[ind];
    e_state.source_valid = true;
    e_state.needs_head_check = true;
    e_state.needs_update_relation = true;
    changed_inds.push_back(ind);
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

void InputState::ensure_larger_than(size_t new_size){
    if(new_size > size()){
        size_t new_cap = std::max(new_size, _capacity + std::min(64,_capacity));
        head_ptrs.conservativeResize(new_cap, n_heads);
        entry_states.reserve(new_cap);
    }
}

bool InputState::validate_head_or_retract(int64_t f_id, size_t change_ind){

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
                    insert_change_dep(f_id, deref_info, ind, true);
                }
                inst_ptr = (Fact*) mbr_ptr->get_ptr();
            }
            if(inst_ptr != nullptr){
                const DerefInfo& deref_info = deref_infos[-1];
                Member* mbr_ptr = deref_once(inst_ptr, deref_info);
                insert_change_dep(f_id, deref_info, ind, false);
                return (Item*) mbr_ptr;
            }else{
                return nullptr;
            }
        }
    }
}

void InputState::update(){
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
CORGI_Node::CORGI_Node(CORGI_Graph* graph, Literal* literal, std::vector<CORGI_IO*> inputs) :
    graph(graph), literal(literal), inputs(inputs) {
    // n_vars = literal->var_inds.size();
    // var_inds = literal->var_inds;
    for(size_t i=0; i < inputs.size(); i++){
        input_states.push_back(InputState(this, i));
        inputs[i]->downstream_input_states.push_back(&input_states[i]);
        outputs.push_back(std::make_unique<CORGI_IO>(this));
    }
}

void CORGI_Node::update_alpha_matches_func(){
    InputState& input_state = input_states[0];
    Func* func = (Func*) literal->obj.get();
    size_t n_heads = func->head_infos.size();
    for(auto& change_ind : input_state.changed_inds){
        InputEntryState& e_state = input_state.entry_states[change_ind];
        bool is_match = (func->call_heads(input_state.head_ptrs(change_ind, n_heads)) == CFSTATUS_TRUTHY) ^ literal->negated;
        e_state.true_count = int64_t(is_match);
    }
}

void CORGI_Node::update_beta_matches_func(){
    for(size_t i=0; i < input_states.size(); i++){
        InputState& input_state = input_states[i];
    }

}

void CORGI_Node::update_output_changes(){
    for(size_t i=0; i < input_states.size(); i++){
        InputState& input_state = input_states[i];
        for(size_t k=0; k < input_state.entry_states.size(); k++){
            InputEntryState& e_state = input_state.entry_states[k];
            CORGI_IO* output = outputs[k];
            bool true_is_nonzero = (e_state.true_count > 0);
            e_state.true_ever_nonzero |= true_is_nonzero;
            
            // Skip other checks if this fact has never been a match.
            if(!e_state.true_ever_nonzero) continue;

            // If the fact is not present in the output and is now a match,
            //  add it to the output and signal the downstream nodes.
            if(!e_state.present_in_output && true_is_nonzero){
                e_state.present_in_output = true;
                auto output_ind = output->add(e_state.f_id);
                e_state.output_ind = output_ind;
                output->downstream_signal_add(e_state.f_id);
            
            // If the fact is present in the output and is now not a match,
            //  add it to the output and signal the downstream nodes.
            }else if(e_state.present_in_output && !true_is_nonzero){
                e_state.present_in_output = false;
                outputs[k]->remove_at(e_state.output_ind);
                e_state.output_ind = -1;
                output->downstream_signal_remove(e_state.f_id);
            }
        }
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
            root_inputs[type_index] = std::make_unique<CORGI_IO>();
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

std::vector<size_t> CORGI_Graph::_get_degree_order(Logic* logic){
    // Order vars by decreasing beta degree --- the number of other 
    // vars that they share beta literals with. Implements heuristic
    // that the most beta-constrained nodes are matched first. 
    auto has_pairs = Eigen::Tensor<bool, 2, Eigen::RowMajor>(
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
    Eigen::Tensor<size_t, 1, Eigen::RowMajor> degree = has_pairs.sum(1);
    Eigen::Tensor<size_t, 1, Eigen::RowMajor> degree_order = degree.argsort(-1);
    return degree_order.matrix().cast<size_t>().eval().vector();
}

void CORGI_Graph::_add_literal(Literal* lit, std::vector<VarFrontier>& frontiers){
    // Let the inputs to the node be the outputs of the frontiers
    //  of the literals' vars.
    std::vector<CORGI_IO*> inputs = {};
    for(size_t i=0; i < lit->var_inds.size(); i++){
        VarFrontier& frontier = frontiers[lit->var_inds[i]];
        inputs.push_back(frontier.output);
    }

    // Make the new node and add it to the graph.
    std::unique_ptr<CORGI_Node> node = //
        std::make_unique<CORGI_Node>(this, lit, inputs);
    nodes_by_nargs[lit->var_inds.size()].push_back(node.get());
    nodes.push_back(std::move(node));

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
    for(size_t i=0; i < logic->vars.size(); i++){
        CRE_Type* var_type = logic->vars[i]->base_type;
        var_frontiers[i] = VarFrontier(get_root_io(var_type));
    }
    _add_logic(logic, var_frontiers);

    // TODO: Do I need to build var_end_join_ptrs?
}

CORGI_IO* CORGI_Graph::get_root_io(CRE_Type* type) const{
    int32_t type_index = type->type_index;
    if(type_index >= root_inputs.size()){
        root_inputs.resize(type_index + 1, nullptr);
    }
    // if(root_inputs[type_index] == nullptr){
    //     root_inputs[type_index] = std::make_unique<CORGI_IO>();
    // }
    return root_inputs[type_index].get();
}

void CORGI_Graph::parse_change_events(){
    for(size_t i=change_head; i < fact_set->change_queue.size(); i++){
        ChangeEvent& change_event = &fact_set->change_queue[i];
        Fact* fact = fact_set->get(change_event.f_id);
        
        // Modifies route directly to their dependencies 
        auto change_dep_it = change_dep_map.find(change_event);
        std::vector<ChangeDep>& change_deps = change_dep_it->second;
        for(ChangeDep& change_dep : change_deps){
            InputState* input_state = change_dep.input_state;
            input_state->signal_modify(change_dep.ind, change_dep.is_deref_support);
        }

        // DECLARE / RETRACT route to their root inputs and propogate
        //  down the CORGI graph.
        if(change_event.change_kind == CHANGE_KIND_DECLARE || 
           change_event.change_kind == CHANGE_KIND_RETRACT){

            auto add_to_root_io = ([&](CORGI_IO* root_io){
                // Skip if the root io is nullptr.
                if(root_io == nullptr) return;
                if(change_event.change_kind == CHANGE_KIND_DECLARE){
                    root_io->add(change_event.f_id);
                    root_io->downstream_signal_add(change_event.f_id);
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
        node->update();
    }
}

} // namespace cre

