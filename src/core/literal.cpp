#include "../include/literal.h"
#include "../include/fact.h"
#include "../include/func.h"
#include "../include/alloc_buffer.h"
#include <regex>
#include <algorithm>

struct Fact;
struct Var;

namespace cre {

void Literal_dtor(const CRE_Obj* x){
	CRE_Obj_dtor(x);
}


Literal::Literal(CRE_Obj* obj, bool negated) :
	obj(obj), negated(negated) {

	switch(obj->get_t_id()){
	case T_ID_FUNC:
	{
		kind = LIT_KIND_FUNC;

		// TODO: This should just get stored in Funcs
		// const std::map<void*, size_t> base_var_map;
		Func* func = (Func*) obj;			
		for(auto hrng : func->head_ranges){
			// for(uint16_t j=hrng.start; j < hrng.end; ++j){
			HeadInfo& head_info = func->head_infos[hrng.start];		
			vars.push_back(head_info.var_ptr->base);		
		}
		break;
	}
	case T_ID_FACT:
	{
		kind = LIT_KIND_FACT;
		for(auto mbr: (Fact*) obj){
			if(mbr.get_t_id() == T_ID_VAR){
				Var* v = (Var*) mbr._as<Var*>();
				bool found = false;
				for(auto var : vars){
					if(bases_semantically_equal(var, v)){found = true; break;}
				}
				if(!found) vars.push_back(v->base);
			}
		}
		break;
	}
	case T_ID_VAR:
	{
		kind = LIT_KIND_VAR;
		vars.push_back((Var*) obj);
		break;
	}
	default:
		std::stringstream ss;
		ss << "Literal Constructor Takes Func, Fact, or Var, got: " << obj;
		std::invalid_argument(ss.str());
	}
}

ref<Literal> new_literal(CRE_Obj* obj, bool negated, AllocBuffer* buffer){
	auto [addr, did_malloc] =  alloc_cre_obj(sizeof(Literal), &Literal_dtor, T_ID_LITERAL, buffer);
	Literal* lit = new (addr) Literal(obj, negated);
	return lit;
}


std::string Literal::to_string(uint8_t verbosity) {
	// std::stringstream ss;

	// if(negated){
	// 	ss << "~";
	// }

	switch(kind){
	case LIT_KIND_FACT:
	{
		Fact* fact = (Fact*) obj.get();
		return fmt::format("{}{}", negated ? "~" :"", fact->to_string(verbosity));
	}
	case LIT_KIND_FUNC:
	{
		Func* func = (Func*) obj.get();
		if(func->origin_data != nullptr && func->origin_data->negated_shorthand_template.size() > 0){
			return func->to_string(verbosity, negated);
		}else{
			return fmt::format("{}{}", negated ? "~" :"", func->to_string(verbosity));
		}
		break;
	}
	// TODO: Perhaps this should not be an option?
	case LIT_KIND_VAR:
	{
		Var* var = (Var*) obj.get();
		return fmt::format("{}{}", negated ? "~" :"", var->to_string());
	}
	default:
		std::runtime_error("Literal Kind Unkown.");
	}
	return "";
}

std::ostream& operator<<(std::ostream& out, Literal* lit){
return out << lit->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Literal> lit){
	return out << lit.get()->to_string();
}


CRE_Type* Literal::eval_type() const {
	if(is_func()){
		return ((Func*) obj.get())->return_type;
	} else {
		throw std::runtime_error("Cannot evaluate type of non-func literal");
	}
	return nullptr;
}
uint16_t Literal::eval_t_id() const {
	return eval_type()->get_t_id();
}

ref<Literal> Literal::copy(Mapping* var_mapping, AllocBuffer* alloc_buffer){
	bool is_func = kind == LIT_KIND_FUNC || kind == LIT_KIND_EQ;
	bool is_fact = kind == LIT_KIND_FACT;
	bool is_var = kind == LIT_KIND_VAR;

    ref<CRE_Obj> new_obj = nullptr;
	if(is_func){
		Func* func = (Func*) obj.get();
		Item* args = (Item*) alloca(func->n_args*sizeof(Item));
		bool did_substitute = false;
		for(size_t i=0; i < func->n_args; ++i){
			auto head_range = func->head_ranges[i];
			Var* var_i = func->head_infos[head_range.start].var_ptr->base;

			Item mapped_val = var_mapping->get(var_i->alias);
			// cout << "BASE: " << hi.var_ptr->base << " ARG: " << arg << endl;
			if(mapped_val.is_undef()){
				new (args + i) Item(var_i);
			}else{
				if(mapped_val.get_t_id() == T_ID_VAR &&
				   uint64_t(mapped_val._as<Var*>()) != uint64_t(var_i)){
					did_substitute = true;
				}
				new (args + i) Item(mapped_val);
			}
		}
		
		// If any substitution was made then make a copy of the underlying func
		if(did_substitute){
			new_obj = func->compose_args(args, func->n_args, alloc_buffer);
		}else{
			new_obj = ref<CRE_Obj>(func);
		}
		// new_obj = func->compose_args(args, alloc_buffer);
	}else if(is_fact){
		// Fact* fact = (Fact*) obj.get();
		// new_obj = fact->copy(new_vars, alloc_buffer);
		throw std::runtime_error("Not implemented. Copy Literals of Fact type.");
	}else if(is_var){
		throw std::runtime_error("Not implemented. Copy Literals of Var type.");
	}

	ref<Literal> new_lit = new_literal(new_obj, negated, alloc_buffer);

	new_lit->structure_weight = structure_weight;
	new_lit->match_weight = match_weight;
	return new_lit;
}

ref<Literal> Literal::copy(AllocBuffer* alloc_buffer){
	return copy(nullptr, alloc_buffer);
}

bool literals_equal(const Literal* lit1, const Literal* lit2, bool semantic, bool castable){
	if(lit1->kind != lit2->kind) return false;
	if(lit1->negated != lit2->negated) return false;

	switch(lit1->kind){
	case LIT_KIND_FACT:
	{
		Fact* fact1 = (Fact*) lit1->obj.get();
		Fact* fact2 = (Fact*) lit2->obj.get();
		return facts_equal(fact1, fact2, semantic);
		break;
	}
	case LIT_KIND_FUNC:
	{
		Func* func1 = (Func*) lit1->obj.get();
		Func* func2 = (Func*) lit2->obj.get();
		return funcs_equal(func1, func2, semantic, castable);
		break;
	}
	case LIT_KIND_VAR:
	{
		Var* var1 = (Var*) lit1->obj.get();
		Var* var2 = (Var*) lit2->obj.get();
		return vars_equal(var1, var2, true, semantic, castable);
		break;
	}
	case LIT_KIND_EQ:
	{
		Func* func1 = (Func*) lit1->obj.get();
		Func* func2 = (Func*) lit2->obj.get();
		return !semantic || funcs_equal(func1, func2, semantic, castable);
		break;
	}
	default:
		return false;
	}
}

bool Literal::operator==(const Literal& other) const {
	return literals_equal((Literal*) this, (Literal*) &other, true, false);
}


} // NAMESPACE_END(cre)
