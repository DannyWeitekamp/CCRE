#include "../include/alloc_buffer.h"
#include "cre_obj.h"           // for CRE_Obj, alloc_cre_obj, CRE_Obj_dtor
#include "types.h"             // for T_ID
#include <cstddef>
#include <sstream>              // for stringstream
#include <stdexcept>           // for runtime_error
#include <vector>
#include "../include/logic.h"  // for Logic
#include "../include/t_ids.h"  // for T_ID_LOGIC
#include "../include/types.h"    // for Var, VAR_KIND_ABSOLUTE, VAR_KIND_BOUND, VAR_KIND_OPTIONAL, VAR_KIND_EXIST, VAR_KIND_NOT
#include "../include/var.h"    // for Var, VAR_KIND_ABSOLUTE, VAR_KIND_BOUND, VAR_KIND_OPTIONAL, VAR_KIND_EXIST, VAR_KIND_NOT
#include "../include/fact.h"   // for Fact  
#include "../include/func.h"   // for Func  
#include "../include/builtin_funcs.h"   // for Equals
#include "../include/hash.h"   // for HashSet

namespace cre {

void Logic_dtor(const CRE_Obj* x) {
	Logic* logic = (Logic*) x;
	// Vector of refs will automatically handle reference counting
	CRE_Obj_dtor(x);
}

Logic::Logic(uint8_t kind) :
	kind(kind),
	is_pure_conj(kind == CONDS_KIND_AND),
	is_pure_disj(kind == CONDS_KIND_OR),
	is_pure_const_or(kind == CONDS_KIND_OR)
{}

ref<Logic> new_logic(uint8_t kind, AllocBuffer* buffer) {
	auto [addr, did_malloc] = alloc_cre_obj(sizeof(Logic), &Logic_dtor, T_ID_LOGIC, buffer);
	Logic* logic = new (addr) Logic(kind);
	return logic;
}

void Logic::_insert_const(const Item& arg) {
    // size_t s = items.size();
    const_item_inds.push_back(items.size());
    items.push_back(arg);
    // lit_semantics.push_back(LiteralSemantics(LIT_SEMANTICS_OR_CONSTS, s, s));
    lit_semantics.push_back(LiteralSemantics());
    
}

void Logic::_insert_var(Var* var, bool part_of_item, uint8_t kind) {
    // cout << "INSERT VAR: " << var->get_alias_str() << ", PART OF ITEM: " << part_of_item << endl;
    auto it = var_map.find(var);
    if(it == var_map.end()) {
        // cout << "  NOT IN MAP" << endl;
        if(kind == uint8_t(-1)) kind = var->kind;
        
        VarInfo info = VarInfo(var, kind, items.size());
        if(part_of_item){
            info.item_inds.push_back(items.size());
        }

        // Ensure that the var has a unique alias
        if(var->alias.is_undef() && ext_locate_var_alias != nullptr){
            ext_locate_var_alias(var);
            // cout << "EXT LOC VAR ALIAS: " << var->get_alias_str() << endl;
        }
        if(var->alias.is_undef()){
            find_unique_var_alias(var, var_map);
            // cout << "FIND UNIQUE VAR ALIAS: " << var->get_alias_str() << endl;
        }

        // Insert the var into the var_map
        auto [_it, inserted] = var_map.insert({var, info});
        var_map_iters.push_back(_it);

        
    }else{
        // cout << "  IN MAP" << endl;
        if(part_of_item){
            VarInfo& info = it->second;
            info.item_inds.push_back(items.size());
        }
    }
}

void Logic::_insert_literal(ref<Literal> lit, const LiteralSemantics& semantics) {
    // cout << "INSERT LITERAL:" << lit << endl;
    if(lit->vars.size() == 0){
        _insert_const(Item(lit));
        return;
    }

    for(auto var : lit->vars) {
        _insert_var(var, true);
    }
    items.push_back(Item(lit));
    lit_semantics.push_back(semantics);
}

void Logic::_extend_same_kind(ref<Logic> logic) {
    size_t s = items.size();
    for(size_t i=0; i<logic->items.size(); i++){
        LiteralSemantics semantics = logic->lit_semantics[i];
        if(semantics.first_item != -1) semantics.first_item += s;
        if(semantics.last_item != -1) semantics.last_item += s;
        _insert_arg(logic->items[i], semantics);
    }
}

void Logic::_insert_other_kind(ref<Logic> logic) {
    // cout << "INSERT OTHER KIND: " << logic->to_string() << "SIZE:" << logic->vars.size() << endl;
    for(size_t i=0; i<logic->vars.size(); i++){
    // for(auto inner_var : item_logic->vars){
        Var* inner_var = logic->vars[i];
        if(i >= logic->n_abs_vars && 
            inner_var->kind == VAR_KIND_ABSOLUTE){
            _insert_var(inner_var, true, VAR_KIND_OPTIONAL);
        }else{
            _insert_var(inner_var, true);
        }
    }
    if(logic->is_pure_const_or){
        size_t s = items.size();
        lit_semantics.push_back(LiteralSemantics(LIT_SEMANTICS_OR_CONSTS, s, s + 1));
    }else{
        lit_semantics.push_back(LiteralSemantics());
    }
    items.push_back(Item(logic));

    if(logic->kind == CONDS_KIND_OR){
        is_pure_conj = false;
    }else{
        is_pure_disj = false;
    }
}

ref<Logic> fact_to_conjunct(Fact* fact, Var* var, AllocBuffer* alloc_buffer) {
    FactType* f_type = fact->type;

    ref<Var> subj_var = var == nullptr ? new_var(Item(), f_type) : ref<Var>(var);

    size_t L = 0;
    for (uint16_t i=0; i<fact->length; i++){
        if(fact->get(i).get_t_id() != T_ID_UNDEF) L++;
    }

    ref<Logic> conjuct = new_logic(CONDS_KIND_AND, alloc_buffer);
    
    for (uint16_t i=0; i<fact->length; i++){
        Member mbr = fact->get(i);
        if(mbr.get_t_id() == T_ID_UNDEF){
            continue;
        }
        ref<Var> mbr_var;
        if(f_type != nullptr){
            // cout << "DEREF KIND ATTR" << endl;
            DerefInfo* deref = (DerefInfo*) alloca(sizeof(DerefInfo));
            deref->deref_type = f_type->get_item_type(i);
            deref->mbr_ind = i;
            deref->deref_kind = DEREF_KIND_ATTR;
            mbr_var = subj_var->_extend_unsafe(deref, 1);
        }else{
            mbr_var = subj_var->extend_item(i);
        }
        // cout << "MBR_VAR: " << mbr_var << endl;
        ref<Literal> mbr_lit = new_literal(Equals->compose(mbr_var, mbr));
        conjuct->_insert_literal(mbr_lit, LiteralSemantics(LIT_SEMANTICS_FACT, 0, L - 1));
    }
    conjuct->_finalize();
    return conjuct;
}

ref<Logic> distribute_OR_const(Logic* disj, Var* var, AllocBuffer* alloc_buffer) {
    
    ref<Logic> new_disj = new_logic(CONDS_KIND_OR, alloc_buffer);
    size_t L = disj->items.size();
    for(size_t i=0; i<disj->items.size(); i++){
        ref<Literal> lit = new_literal(Equals->compose(var, disj->items[i]));
        new_disj->_insert_literal(lit, LiteralSemantics(LIT_SEMANTICS_OR_CONSTS, 0, L-1));
    }
    new_disj->_finalize();
    return new_disj;
}

// void Logic::_insert_fact_as_literals(ref<Fact> fact) {
    
//     size_t s = items.size();
//     FactType* f_type = fact->type;
//     ref<Var> subj_var = new_var(Item(), f_type);

//     size_t L = 0;
//     for (uint16_t i=0; i<fact->length; i++){
//         if(fact->get(i).get_t_id() != T_ID_UNDEF) L++;
//     }
    
//     for (uint16_t i=0; i<fact->length; i++){
//         Member mbr = fact->get(i);
//         if(mbr.get_t_id() == T_ID_UNDEF){
//             continue;
//         }
//         ref<Var> mbr_var;
//         if(f_type != nullptr){
//             cout << "DEREF KIND ATTR" << endl;
//             DerefInfo* deref = (DerefInfo*) alloca(sizeof(DerefInfo));
//             deref->deref_type = f_type->get_item_type(i);
//             deref->mbr_ind = i;
//             deref->deref_kind = DEREF_KIND_ATTR;
//             mbr_var = subj_var->_extend_unsafe(deref, 1);
//         }else{
//             mbr_var = subj_var->extend_item(i);
//         }
//         cout << "MBR_VAR: " << mbr_var << endl;
//         ref<Literal> mbr_lit = new_literal(Equals->compose(mbr_var, mbr));
//         _insert_literal(mbr_lit, LiteralSemantics(LIT_SEMANTICS_FACT, s, s + L - 1));
//     }
// }


void Logic::_insert_arg(const Item& arg, LiteralSemantics semantics) {
    // cout << "ARG T_ID: " << obj->get_t_id() << endl;
    bool was_const = false;
    switch(arg.get_t_id()) {
    case T_ID_LITERAL:
        _insert_literal(arg._as<Literal*>(), semantics);
        break;
    case T_ID_FUNC:
        _insert_literal(new_literal(arg._as<CRE_Obj*>()), semantics);
        break;
    case T_ID_VAR:
        if(arg._as<Var*>()->bound_obj.is_undef()){
            _insert_var(arg._as<Var*>(), false);
        }else{
            ref<Logic> bound_logic = arg._as<Var*>()->bound_obj._as<Logic*>();
            if(kind == bound_logic->kind){
                _extend_same_kind(bound_logic);
            }else{
                _insert_other_kind(bound_logic);
            }
        }
        break;
    case T_ID_LOGIC:
    {
        Logic* item_logic = arg._as<Logic*>(); 
        if(kind == item_logic->kind){
            _extend_same_kind(item_logic);
        }else{
            _insert_other_kind(item_logic);
        }
        break;
    }
    case T_ID_FACT:
    {
        Fact* fact = arg._as<Fact*>();
        if(fact->immutable){
            _insert_literal(new_literal((CRE_Obj*) fact), semantics);
        }else{
            ref<Logic> fact_conj = fact_to_conjunct(fact);
            if(kind == CONDS_KIND_AND){
                _extend_same_kind(fact_conj);
            }else{
                _insert_other_kind(fact_conj);
            }
        }
        break;
    }
    default:
        if(t_id_is_primitive(arg.get_t_id())){
            _insert_const(arg);
            was_const = true;
        }else{
            std::stringstream ss;
            ss << "Argument to Logic with type " << arg.get_type() << " is not supported.";
            throw std::invalid_argument(ss.str());
        }
    }
    if(!was_const && kind == CONDS_KIND_OR){
        is_pure_const_or = false;
    }
}

void Logic::_finalize() {
    // When OR() change any absolute vars to optional if they 
    //   do not occur in every item.
    if(kind == CONDS_KIND_OR){
        for(auto [var, info] : var_map){
            if(info.kind == VAR_KIND_ABSOLUTE && 
               info.item_inds.size() != items.size()){
                info.kind = VAR_KIND_OPTIONAL;
            }
        }
    }

    // Put the absolute vars at the beginning of vars
    for(auto it : var_map_iters){
        auto& [var, info] = *it;
        if(info.kind == VAR_KIND_ABSOLUTE){
            n_abs_vars++;
            info.pos = vars.size();
            vars.push_back(var);
        }
    }

    // Put the non-absolute vars at the end of vars
    for(auto it : var_map_iters){
        auto& [var, info] = *it;
        if(info.kind != VAR_KIND_ABSOLUTE){
            info.pos = vars.size();
            vars.push_back(var);
        }
    }

    // Assign the var_inds of each literal
    for(Item& item : items){
        if(item.get_t_id() == T_ID_LITERAL){
            Literal* lit = item._as<Literal*>();
            size_t nv = lit->vars.size();
            cout << "nv: " << nv << endl;
            VarInds& var_inds = lit->var_inds;
            var_inds.reserve(nv);
            cout << "LIT VAR INDS: " << lit->var_inds << " SIZE:" << lit->var_inds.size() << endl;
            // uint16_t* var_inds_buff = (uint16_t*) alloca(nv * sizeof(uint16_t));
            for(size_t i=0; i < nv; i++){
                Var* var = lit->vars[i];
                auto it = var_map.find(var);
                if(it != var_map.end()) {
                    auto& info = it->second;
                    var_inds[i] = info.pos;
                    cout << "WRITE:" << i << " , " << info.pos << endl;
                } else {
                    throw std::runtime_error("Logic::_finalize: variable not found in var_map");
                }
            }
            cout << "LIT VAR INDS: " << lit->var_inds << " SIZE:" << lit->var_inds.size() << endl;
        }
    }


    // cout << "FSIZE:" << vars.size() << "MSIZE:" << var_map.size() << endl;
            // info.kind_pos = abs_vars.size()
            // abs_vars.push_back(var);
            
        // case VAR_KIND_BOUND:
        //     info.kind_pos = bnd_vars.size()
        //     bnd_vars.push_back(var);
        //     break;
        // case VAR_KIND_OPTIONAL:
        //     info.kind_pos = opt_vars.size()
        //     opt_vars.push_back(var);
        //     break;
        // case VAR_KIND_EXIST:
        //     info.kind_pos = ext_vars.size()
        //     ext_vars.push_back(var);
        //     break;
        // case VAR_KIND_NOT:
        //     info.kind_pos = ext_vars.size()
        //     ext_vars.push_back(var);
        //     break;
        // }
    // }

    // for(auto var : abs_vars){
    //     var_map[var].pos = vars.size()
    //     vars.push_back(var);
    // }
    // for(auto var : bnd_vars){
    //     var_map[var].pos = vars.size()
    //     vars.push_back(var);
    // }
    // for(auto var : opt_vars){
    //     var_map[var].pos = vars.size()
    //     vars.push_back(var);
    // }
    // for(auto var : ext_vars){
    //     var_map[var].pos = vars.size()
    //     vars.push_back(var);
    // }
}

void Logic::_ensure_standard_order(){
    if(standard_order.size() == 0 && items.size() != 0){
        standard_order = std::vector<size_t>(items.size(), -1);
        // standard_var_spans.reserve(vars.size());
        std::vector<bool> covered(items.size(), false);

        size_t c = 0;
        // size_t last_end = 0;
        // for(auto var : vars){
        //     VarInfo& info = var_map.at(var);
        //     // size_t start = c;
        //     // cout << "VAR: " << var->get_alias_str() << " Item Inds: " << info.item_inds << endl;
        //     size_t min_ind = std::numeric_limits<size_t>::max();
        //     size_t max_ind = last_end;
        //     for(auto ind : info.item_inds){
        //         // cout << "IND: " << ind << endl;
        //         min_ind = std::min(min_ind, ind);
        //         max_ind = std::max(max_ind, ind);
        //         // if(!covered[ind]){
        //         //     standard_order[c] = ind;
        //         //     covered[ind] = true;
        //         //     c++;
        //         // }
        //     }
        //     if(min_ind > max_ind) min_ind = max_ind;
        //     // cout << "++START: " << min_ind << ", END: " << max_ind << endl;
        //     standard_var_spans.emplace_back(min_ind, max_ind);
        //     last_end = max_ind;
        //     // if(info.item_inds.size() >= 1) last_end = max_ind+1;
        // }
        for(size_t i=0; i<standard_order.size(); i++){
            if(!covered[i]){
                standard_order[c] = i;
                covered[i] = true;
                c++;
            }
            // standard_var_spans.emplace_back(-1, -1);
        }
    }
}

std::string Logic::basic_str() {
	std::stringstream ss;
    ss << (kind == CONDS_KIND_AND ? "AND(" : "OR(");	
    for (size_t i = 0; i < items.size(); ++i) {
        Item& item = items[i];
        // cout << "ITEM T_ID: " << item->get_t_id() << endl;
        switch(item.get_t_id()){
        case T_ID_LITERAL:
            ss << item._as<Literal*>()->to_string();    
            break;
        case T_ID_LOGIC:
            ss << item._as<Logic*>()->to_string();    
            break;
        default:
            cout << "??" << endl;
        }        
        if(i < items.size() - 1) {
            ss << ", ";
        }
    }
    ss << ")";
	return ss.str();
}

size_t Logic::_stream_item(std::stringstream& ss, size_t i, std::string_view indent,
        HashSet<void*>* var_covered, std::vector<bool>& item_covered, size_t& n_items_covered, bool is_first) {
    
    size_t n_adv = 0;
    Item& item = items[i];
    LiteralSemantics semantics = lit_semantics[i];
    // cout << "STREAM ITEM: " << i << ", SEMANTICS: " << semantics.kind << endl;

    
    if(semantics.kind == LIT_SEMANTICS_FACT && item.get_t_id() == T_ID_LITERAL){
        // cout << "+SEMANTIC :" << i << endl;
        
        while(i <= semantics.last_item){
            item_covered[i] = true;
            n_items_covered++;
            item = items[i];
            

            Literal* lit = item._as<Literal*>();
            Func* func = (Func*) lit->obj.get();
            Var* attr_var = func->get(0)->_as<Var*>();
            DerefInfo& deref = attr_var->deref_infos[0];
            Item& attr_val = *func->get(1);
            FactType* fact_type = (FactType*) attr_var->base_type;
            if(i == semantics.first_item){
                if(is_first){
                    ss << fmt::format("{}({}(", attr_var->get_prefix_str(), fact_type->to_string());
                }else{
                    ss << fmt::format("{} == {}(", attr_var->get_alias_str(), fact_type->to_string());
                }
            }
            
            if(deref.deref_kind == DEREF_KIND_ATTR){       
                // cout << "ATTR: " << fact_type->get_item_attr(deref.mbr_ind) << endl;         
                ss << fmt::format("{}={}", 
                    fact_type->get_item_attr(deref.mbr_ind),
                    attr_val.to_string());
            }else{
                // cout << "ITEM: " << deref.deref_kind << endl;
                ss << fmt::format("{}", attr_val.to_string());
            }
            // cout << "LAST ITEM: " << semantics.last_item << ", I: " << i << endl;
            if(i != semantics.last_item) ss << ", ";                    
            i++;
            n_adv++;
        }
        --n_adv;
        ss << ")";
        if(is_first) ss << ")";
        // if(i < L-1) ss << ", ";
    }else if(semantics.kind == LIT_SEMANTICS_OR_CONSTS){
        // cout << "OR CONST SEMANTICS: " << semantics.first_item << ", " << semantics.last_item << endl;
        size_t j =0;
        std::vector<Item>& const_items = items;
        LiteralSemantics& const_semantics = semantics;

        if(item.get_t_id() == T_ID_LOGIC){
            Logic* inner_logic = item._as<Logic*>();
            const_items = inner_logic->items;
            // cout << "SEMANTICS SIZE: " << inner_logic->lit_semantics.size() << endl;
            const_semantics = inner_logic->lit_semantics[0];
        }else{
            j = i;
        }
        while(j <= const_semantics.last_item){
            item_covered[j] = true;
            n_items_covered++;

            Item& const_item = const_items[j];

            Literal* lit = const_item._as<Literal*>();
            Func* func = (Func*) lit->obj.get();
            Var* var = func->get(0)->_as<Var*>();
            Item const_val = *func->get(1);
            if(j == const_semantics.first_item){
                ss << fmt::format("{} == OR(", var->to_string());
            }
            // cout << j << ", " << const_val << endl;
            ss << const_val;
            if(j != const_semantics.last_item) ss << ", ";
            j++;
        }
        if(item.get_t_id() != T_ID_LOGIC){
            n_adv = j-i-1;
        }
        ss << ")";
        // cout << "END SEMANTIC :" << i << ", " << ss.str() << endl;
    }else{
        item_covered[i] = true;
        n_items_covered++;
        
        switch(item.get_t_id()){
        case T_ID_LITERAL:
            // cout << "NORMAL Literal: " << i << endl;
            ss << item._as<Literal*>()->to_string();
            // cout << "END NORMAL Literal: " << i << endl;
            // if(i < L-1) ss << ", ";

            // prev_endl = false;
            // if(mult_vars && 
            //     (i+1 >= end ||
            //      i+1 >= L ||
            //      items[standard_order[i+1]].get_t_id() == T_ID_LOGIC)){
            //     ss << "\n";
            //     prev_endl = true;
            // }
            break;
        case T_ID_LOGIC:
        {
            // cout << "LOGIC: " << i << endl;
            Logic* inner_logic = item._as<Logic*>();
            std::string next_indent = fmt::format("{}{}", indent, indent);
            // ss << indent;
            ss << inner_logic->standard_str(next_indent, indent, var_covered);
            // if(inner_logic->vars.size() > 1) ss << indent;
            // ss << indent << ")";
            // if(i < L-1) ss << ", ";
            // ss << "\n";
            break;
        }
        default:
            // cout << "DEFAULT: " << i << endl;
            ss << item.to_string();
            // if(i < L-1) ss << ", ";
            break;
        }
    }
    return n_adv;
}

size_t _count_vars_covered(std::vector<ref<Var>>& vars, HashSet<void*>* var_covered){
    size_t n_covered = 0;
    for(size_t i=0; i<vars.size(); i++){
        if(var_covered->find((void*) vars[i].get()) != var_covered->end()){
            n_covered++;
        }
    }
    return n_covered;
}
bool _all_vars_covered(Item& item, HashSet<void*>* var_covered){
    switch(item.get_t_id()){
    case T_ID_LOGIC:
    {
        std::vector<ref<Var>>& vars = item._as<Logic*>()->vars;
        return _count_vars_covered(vars, var_covered) == vars.size();
    }        
    case T_ID_LITERAL:
    {
        std::vector<ref<Var>>& vars = item._as<Literal*>()->vars;
        return _count_vars_covered(vars, var_covered) == vars.size();
    }
    default:
        return false;
    }
}

bool _all_true(std::vector<bool> arr){
    for(size_t i=0; i<arr.size(); i++){
        if(!arr[i]) return false;
    }
    return true;
}

std::string Logic::standard_str(
    std::string_view indent, std::string_view prev_indent,
    HashSet<void*>* var_covered) {

    _ensure_standard_order();
    bool finished = false;
    bool is_outermost = false;
    size_t n_items_covered = 0;
    size_t n_vars_covered = 0;
    size_t L = vars.size();
    // size_t n_lines = 0;
    
    std::vector<bool> item_covered = std::vector<bool>(items.size(), false);
    if(var_covered == nullptr){
        var_covered = new HashSet<void*>();
        is_outermost = true;

        for(size_t i=0; i<L; i++){
            Var* v = vars[i];

            // bool var_will_cover = !inner_covers_var && var_covered->find((void*) v) == var_covered->end();            

            // Skip over 
            if(v->kind == VAR_KIND_BOUND){
                var_covered->insert((void*) v);
                ++n_vars_covered;
            }//else if(var_will_cover || v.item_inds.size() >= 1){
                // ++n_lines;
            // }
        }
    }else{
        // n_explicit_vars = L;
        n_vars_covered = _count_vars_covered(vars, var_covered);
    }

    // bool is_multiline = n_lines > 1;

    

    
    // cout << "MV:" << mult_vars << "SIZE:" << vars.size() << endl;
    // if(is_multiline) ss << "\n";
    

    size_t v_ind = 0;
    
    // int64_t start = -1;
    // int64_t end = -1;   

    std::vector<std::string> lines;

    // Handle any const items first
    std::stringstream const_ss;
    for(size_t j=0; j<const_item_inds.size(); j++){
        std::stringstream ss;
        size_t ind = const_item_inds[j];
        if(!item_covered[ind]){
            _stream_item(const_ss, ind, indent, var_covered, item_covered, n_items_covered, false);
            if(j < const_item_inds.size()-1) const_ss << ", ";
        }
    }
    if(const_ss.str().size() > 0) lines.push_back(const_ss.str());

    for(size_t i=0; i<L; i++){

        std::stringstream ss;
        Var* v = vars[i];
        // cout << "VAR: " << v->get_alias_str() << endl;
        VarInfo& info = var_map.at(v);
        bool write_any = false;

        std::stringstream var_ss; 
        std::stringstream lit_ss; 

        bool inner_covers_var = false;
        if(info.item_inds.size() == 1){
            Item& item = items[info.item_inds[0]];
            
            inner_covers_var = item.get_t_id() == T_ID_LOGIC;
            // if(inner_covers_var) cout << "BLAH: " << item << endl;
        }
        // cout << "VAR TO INNER: " << inner_covers_var << " @" << info.item_inds[0] << endl;

        bool var_will_cover = !inner_covers_var && var_covered->find((void*) v) == var_covered->end();
        if(var_will_cover){
            var_covered->insert((void*) v);
            ++n_vars_covered;
            // finished = n_items_covered == items.size() && n_vars_covered == vars.size();
            write_any = true;
        }

        bool first_is_fact_semantics = false;
        for(size_t j=0; j<info.item_inds.size(); j++){
            
            size_t ind = info.item_inds[j];
            Item& item = items[ind];
            
            bool has_var_prereqs = _all_vars_covered(item, var_covered);
            if(!item_covered[ind] && (has_var_prereqs || inner_covers_var)){
                // bool item_is_logic = item.get_t_id() == T_ID_LOGIC;
                // bool item_is_multiline = item_is_logic && item._as<Logic*>()->vars.size() > 1;
                
                // if(item_is_multiline) lit_ss << "\n" << indent;
                if(j==0) first_is_fact_semantics = lit_semantics[ind].kind == LIT_SEMANTICS_FACT;                
                size_t n_adv = _stream_item(lit_ss, ind, indent, var_covered, item_covered, n_items_covered, first_is_fact_semantics && j==0);
                j += n_adv;

                // finished = n_items_covered == items.size() && n_vars_covered == vars.size();
                // if(!finished && item_is_multiline) lit_ss << ")";
                // if(!finished) lit_ss << ", ";
                if(j < info.item_inds.size()-1) lit_ss << ", ";
                // if(!finished && item_is_multiline) lit_ss << "\n";
                write_any = true;
            }
            // if(!all_vars_covered) cout << "NOT ALL VARS COVERED: " << items[ind].get_t_id() << endl;
            // if(!all_vars_covered) cout << "NOT ALL VARS COVERED: " << items[ind] << endl;
        }
        
        // cout << "First is fact semantics: " << first_is_fact_semantics << endl;
        if(var_will_cover){
            if(first_is_fact_semantics){
                var_ss << fmt::format("{}:=", v->get_alias_str());
            }else{
                var_ss << fmt::format("{}:={}, ",  v->get_alias_str(), v->repr(false));
            }
        }
        
        
        // if(is_multiline && write_any) ss << indent;}
        if(write_any){
            ss << var_ss.str() << lit_ss.str();
            lines.push_back(ss.str());
        }
        

        // bool finished = _all_true(item_covered);

        // if(!finished) ss << ", ";
        // if(is_multiline && write_any) ss << "\n";

        if(finished) break;
        
    }

    std::stringstream ss;
    ss << (kind == CONDS_KIND_AND ? "AND(" : "OR(");

    if(lines.size() > 1) ss << "\n";

    for(size_t i=0; i<lines.size(); i++){
        if(lines.size() > 1) ss << indent;
        ss << lines[i];
        if(lines.size() > 1 && i < lines.size()-1) ss << ",\n";
    }

    if(lines.size() > 1) ss << "\n";
    if(lines.size() > 1) ss << prev_indent;
    ss << ")";
    
    if(is_outermost){
        delete var_covered;
    }

    return ss.str();
}

std::string Logic::to_string() {
    return standard_str();
}

std::ostream& operator<<(std::ostream& out, Logic* logic) {
    cout << "LOGIC STR: " << logic->to_string() << endl;
	return out << logic->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Logic> logic) {
	return out << logic->to_string();
}

} // NAMESPACE_END(cre)

