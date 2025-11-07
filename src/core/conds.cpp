#include "../include/conds.h"
#include "../include/alloc_buffer.h"
#include "cre_obj.h"           // for CRE_Obj, alloc_cre_obj, CRE_Obj_dtor
#include "types.h"             // for T_ID
#include <sstream>              // for stringstream
#include <stdexcept>           // for runtime_error
#include "../include/conds.h"  // for Conds
#include "../include/t_ids.h"  // for T_ID_CONDS
#include "../include/var.h"    // for Var, VAR_KIND_ABSOLUTE, VAR_KIND_BOUND, VAR_KIND_OPTIONAL, VAR_KIND_EXIST, VAR_KIND_NOT

namespace cre {

void Conds_dtor(const CRE_Obj* x) {
	Conds* conds = (Conds*) x;
	// Vector of refs will automatically handle reference counting
	CRE_Obj_dtor(x);
}

Conds::Conds(uint8_t kind) :
	kind(kind)
{}

ref<Conds> new_conds(uint8_t kind, AllocBuffer* buffer) {
	auto [addr, did_malloc] = alloc_cre_obj(sizeof(Conds), &Conds_dtor, T_ID_CONDS, buffer);
	Conds* conds = new (addr) Conds(kind);
	return conds;
}


void Conds::_insert_var(Var* var, bool part_of_item, uint8_t kind) {
    // cout << "INSERT VAR: " << var->get_alias_str() << ", PART OF ITEM: " << part_of_item << endl;
    auto it = var_map.find(var);
    if(it == var_map.end()) {

        if(kind == uint8_t(-1)) kind = var->kind;
        
        VarInfo info = VarInfo(var, kind, items.size());
        if(part_of_item){
            info.item_inds.push_back(items.size());
        }
        auto [_it, inserted] = var_map.insert({var, info});
        var_map_iters.push_back(_it);
    }else{
        if(part_of_item){
            VarInfo& info = it->second;
            info.item_inds.push_back(items.size());
        }
    }
}

void Conds::_insert_literal(ref<Literal> lit) {
    // cout << "INSERT LITERAL:" << lit << endl;
    for(auto var : lit->vars) {
        _insert_var(var, true);
    }
    items.push_back(ref<CRE_Obj>(lit.get()));
}

void Conds::_extend_same_kind(ref<Conds> conj) {
    for(auto c_item : conj->items){
        _insert_arg(c_item);
    }
}


void Conds::_insert_arg(CRE_Obj* obj) {
    // cout << "ARG T_ID: " << obj->get_t_id() << endl;

    switch(obj->get_t_id()) {
    case T_ID_LITERAL:
        _insert_literal(ref<Literal>((Literal*) obj));
        break;
    case T_ID_FUNC:
        _insert_literal(new_literal(obj));
        break;
    case T_ID_FACT:
        _insert_literal(new_literal(obj));
        break;
    case T_ID_VAR:
        _insert_var((Var*) obj, false);
        break;
    case T_ID_CONDS:
    {
        Conds* item_conds = (Conds*) obj; 
        if(kind == item_conds->kind){
            _extend_same_kind(item_conds);
        }else{
            for(size_t i=0; i<item_conds->vars.size(); i++){
            // for(auto inner_var : item_conds->vars){
                Var* inner_var = item_conds->vars[i];
                if(i >= item_conds->n_abs_vars && 
                   inner_var->kind == VAR_KIND_ABSOLUTE){
                    _insert_var(inner_var, true, VAR_KIND_OPTIONAL);
                }else{
                    _insert_var(inner_var, true);
                }
                
            }
            items.push_back(obj);
        }
        break;
    }
    default:
        std::stringstream ss;
        ss << "Argument to Conds with type " << obj->get_t_id() << " is not supported.";
        throw std::invalid_argument(ss.str());
    }
}

void Conds::_finalize() {
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

    for(auto it : var_map_iters){
        auto [var, info] = *it;
        if(info.kind == VAR_KIND_ABSOLUTE){
            n_abs_vars++;
            info.pos = vars.size();
            vars.push_back(var);
        }
    }

    for(auto it : var_map_iters){
        auto [var, info] = *it;
        if(info.kind != VAR_KIND_ABSOLUTE){
            info.pos = vars.size();
            vars.push_back(var);
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

void Conds::_ensure_standard_order(){
    if(standard_order.size() == 0 && items.size() != 0){
        standard_order = std::vector<size_t>(items.size(), -1);
        standard_var_spans.reserve(vars.size());
        std::vector<bool> covered(items.size(), false);

        size_t c = 0;
        for(auto var : vars){
            VarInfo& info = var_map.at(var);
            size_t start = c;
            // cout << "Item Inds: " << info.item_inds << endl;
            for(auto ind : info.item_inds){
                if(!covered[ind]){
                    standard_order[c] = ind;
                    covered[ind] = true;
                    c++;
                }
            }
            // cout << "++START: " << start << ", END: " << c << endl;
            standard_var_spans.emplace_back(start, c);
        }
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

std::string Conds::basic_str() {
	std::stringstream ss;
    ss << (kind == CONDS_KIND_AND ? "AND(" : "OR(");	
    for (size_t i = 0; i < items.size(); ++i) {
        CRE_Obj* item = items[i].get();
        // cout << "ITEM T_ID: " << item->get_t_id() << endl;
        switch(item->get_t_id()){
        case T_ID_LITERAL:
            ss << ((Literal*) item)->to_string();    
            break;
        case T_ID_CONDS:
            ss << ((Conds*) item)->to_string();    
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

std::string Conds::standard_str(std::string_view indent, HashSet<Var*>* covered) {
    _ensure_standard_order();

    bool is_outermost = false;
    if(covered == nullptr){
        covered = new HashSet<Var*>();
        is_outermost = true;
    }

    std::stringstream ss;
    ss << (kind == CONDS_KIND_AND ? "AND(" : "OR(");

    bool mult_vars = vars.size() > 1;
    // cout << "MV:" << mult_vars << "SIZE:" << vars.size() << endl;
    if(mult_vars) ss << "\n";
    bool prev_endl = mult_vars;        

    size_t v_ind = 0;
    size_t L = items.size();
    auto [start, end] = standard_var_spans[v_ind];
    // for(size_t j : standard_order){
    for(size_t i=0; i<L; i++){
        if(prev_endl) ss << indent;

        CRE_Obj* item = items[standard_order[i]].get();
        while(item->get_t_id() != T_ID_CONDS &&
              i >= start && i < end && v_ind < vars.size()){
            // cout << "V_IND: " << v_ind << "SIZE" << vars.size() << endl;
            Var* v = vars[v_ind];

            if(!covered->contains(v)){
                ss << fmt::format("{}:=Var({})", v->get_alias_str(), v->base_type->to_string());
                if(start != end) ss << ", ";
                covered->insert(v);
            }

            ++v_ind;
            if(v_ind < standard_var_spans.size()){
                std::tie(start, end) = standard_var_spans[v_ind];    
            }else{
                start = -1;
                end = -1;
            }
            
        }
        
        switch(item->get_t_id()){
        case T_ID_LITERAL:
            ss << ((Literal*) item)->to_string();
            if(i < L-1) ss << ", ";

            prev_endl = false;
            if(mult_vars && 
                ((i+1 >= start && i+1 < end) ||
                  i+1 >= L ||
                  items[standard_order[i+1]]->get_t_id() == T_ID_CONDS)){
                ss << "\n";
                prev_endl = true;
            }
            break;
        case T_ID_CONDS:
        {
            Conds* inner_conds = (Conds*) item;
            std::string next_indent = fmt::format("{}{}", indent, indent);
            ss << inner_conds->standard_str(next_indent, covered);
            if(inner_conds->vars.size() > 1) ss << indent;
            ss << ")";
            if(i < L-1) ss << ", ";
            ss << "\n";
            prev_endl = true;
            break;
        }
        default:
            ss << "??";
            if(i < L-1) ss << ", ";
            prev_endl = false;
            break;
        }
    }
    
    if(is_outermost){
        ss << ")";
        delete covered;
    }

    return ss.str();
}

std::string Conds::to_string() {
    return standard_str();
}

std::ostream& operator<<(std::ostream& out, Conds* conds) {
	return out << conds->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Conds> conds) {
	return out << conds->to_string();
}

} // NAMESPACE_END(cre)

