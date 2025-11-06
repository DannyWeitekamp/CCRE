#include "../include/literal.h"
#include "../include/fact.h"
#include "../include/func.h"
#include "../include/alloc_buffer.h"

struct Fact;
struct Var;

namespace cre {

	void Literal_dtor(const CRE_Obj* x){
		CRE_Obj_dtor(x);
	}


	Literal::Literal(CRE_Obj* obj, bool negated) :
		obj(obj), negated(negated)
	{
		switch(obj->get_t_id()){
		case T_ID_FUNC:
		{
			kind = LIT_KIND_FUNC;

			// TODO: This should just get stored in Funcs
			// const std::map<void*, size_t> base_var_map;
			Func* func = (Func*) obj;			
			for(auto hrng : func->head_ranges){
				// for(uint16_t j=hrng.start; j < hrng.end; ++j){
				HeadInfo head_info = func->head_infos[hrng.start];		
				vars.push_back(head_info.var_ptr->base);		
			}
			break;
		}
		case T_ID_FACT:
		{
			kind = LIT_KIND_FACT;
			for(auto mbr: (Fact*) obj){
				if(mbr.get_t_id() == T_ID_VAR){
					vars.push_back((Var*) mbr._as<Var*>());		
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



	std::string Literal::to_string() {
		std::stringstream ss;

		if(negated){
			ss << "~";
		}

		switch(kind){
		case LIT_KIND_FACT:
			ss << (Fact*) obj.get();
			break;
		case LIT_KIND_FUNC:
			ss << (Func*) obj.get();
			break;
		case LIT_KIND_VAR:
			ss << (Var*) obj.get();
			break;
		default:
			std::runtime_error("Literal Kind Unkown.");
		}

		return ss.str();
	}

	std::ostream& operator<<(std::ostream& out, Literal* lit){
	return out << lit->to_string();
	}

	std::ostream& operator<<(std::ostream& out, ref<Literal> lit){
		return out << lit.get()->to_string();
	}



} // NAMESPACE_END(cre)
