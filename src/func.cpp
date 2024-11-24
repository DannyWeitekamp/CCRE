#include <vector>
#include <bit>
#include <cstdint>
#include <sstream> 
#include <cstring>
#include <iostream>
#include <algorithm>
#include <map>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/args.h>
#include "../include/intern.h"
#include "../include/func.h"
#include "../include/var.h"



void Func_dtor(const CRE_Obj* x){
	Func* func = (Func*) x;
	delete func;
}

std::string make_default_template(
	const std::string_view& name,
	size_t n_args){

	std::vector<std::string> brackets(n_args, "{}");
	return fmt::format(
		"{}({})", name,
		fmt::join(brackets, ", ")
	);

}


//------------------------------------------------------------
// new_func()
ref<Func> new_func(void* cfunc_ptr, size_t n_args,  OriginData* origin_data){
	Func* func = _alloc_func(n_args);
	ref<Func> nf = new (func) Func(cfunc_ptr, n_args, origin_data);
	return nf;
}

// ----------------------------------------------------------
//  define_func()

void _init_arg_specs(Func* func, const std::vector<CRE_Type*>& arg_types){
	size_t n_args = arg_types.size();

	std::vector<ArgInfo>& root_arg_infos = func->root_arg_infos;
	root_arg_infos.reserve(n_args);

 	std::vector<HeadInfo>& head_infos = func->head_infos;
	head_infos.reserve(n_args);

	std::vector<HeadRange>& head_ranges = func->head_ranges;
	head_ranges.reserve(n_args);

	// for(auto arg_typqe : arg_types){
	uint64_t offset = 0;
	for(uint32_t i =0; i< arg_types.size(); ++i){
		CRE_Type* arg_type = arg_types[i];

		auto alias = intern(fmt::format("x{}", i));
		ref<Var> var = new_var(alias, arg_type);

		func->set(i, var.get());

		// ArgInfo
		// uint16_t t_id = arg_type->t_id;
		root_arg_infos.emplace_back(
			arg_type, func->get(i), ARGINFO_VAR, false);

		// Head Info
		HeadInfo hi;
		hi.cf_ptr = func;
		hi.var_ptr = var.get();
		hi.arg_ind = i;
		hi.kind = ARGINFO_VAR;
		hi.has_deref = false;

		hi.base_type = arg_type;
		hi.head_type = arg_type;

		// TODO: Make Offsets
		hi.arg_data_ptr = nullptr;
		hi.head_data_ptr = nullptr;
		hi.head_size = 0;
		hi.ref_kind = 0; // Necessary ? 

		head_infos.push_back(hi);

		// Head Range
		head_ranges.emplace_back(i, i+1);
	}
 }

ref<Func> define_func(
		const std::string_view& name, 
		void* cfunc_ptr,
		CRE_Type* ret_type,
		const std::vector<CRE_Type*>& arg_types,
		const std::string_view& expr_template,
		const std::string_view& shorthand_template){

	size_t n_args = arg_types.size();

	OriginData* origin_data = new OriginData();
	origin_data->name = name;
	if(expr_template.size() == 0){
		origin_data->expr_template = make_default_template(name, n_args);
	}else{
		origin_data->expr_template = expr_template;
	}

	if(shorthand_template.size() == 0){
		origin_data->shorthand_template = make_default_template(name, n_args);
	}else{
		origin_data->shorthand_template = shorthand_template;
	}

	ref<Func> func = new_func(cfunc_ptr, n_args, origin_data);//new Func(origin_data, cfunc_ptr);
	func->dtor = &Func_dtor;

	origin_data->func = func.get();

	_init_arg_specs(func, arg_types);

	return func;
}


// -------------------------------------------------------------
// to_string()

std::string resolve_template(Func* func, uint8_t verbosity){
	OriginData* od = func->origin_data;
	if(verbosity == LOW_VERBOSITY && od->shorthand_template.size() > 0){
		return od->shorthand_template;
	}else if(od->expr_template.size() > 0){
		return od->expr_template;
	}else{
		throw std::runtime_error("Func has no template.");
	}
}




std::string Func::to_string(uint8_t verbosity){
	using fmt_args_t = fmt::dynamic_format_arg_store<fmt::format_context>;
	using stack_tuple = std::tuple<Func*, int, fmt_args_t*> ;

	OriginData* od = this->origin_data;
	bool use_derefs = true;
	bool use_shorthand = verbosity == LOW_VERBOSITY && od->shorthand_template.size() > 0;
	std::string ignore_pattern = "";

	std::vector<stack_tuple> stack = {};
	Func* cf = (Func*) this;
	int i = 0;
	auto arg_strs = new fmt_args_t();
	std::string s;
	bool keep_looping = true;	
	while(keep_looping){
		if(i < cf->root_arg_infos.size()){
			auto& arg_info = cf->root_arg_infos[i];
			if(arg_info.kind == ARGINFO_FUNC){
				// Recurse Case: Func
				stack.emplace_back(stack_tuple(cf, i+1, arg_strs));

				cf = (*arg_info.ptr).as_func();
				arg_strs = new fmt_args_t();
				i =0;
			}else{
				// Terminal Case: Var/Const
				if(arg_info.kind == ARGINFO_VAR){
					Var* var = (*arg_info.ptr).as_var();
					if(use_derefs){
						arg_strs->push_back(var->to_string());
					}else{
						arg_strs->push_back(var->alias.to_string());
					}
				}else if(arg_info.kind == ARGINFO_CONST){
					Item& item = *cf->get(i);
					arg_strs->push_back(item.to_string());
				}else{
					throw std::runtime_error("Bad arginfo type.");
				}
				++i;
			}
		}

		while(i >= cf->root_arg_infos.size()){
			auto nd = cf->origin_data;
			// bool should_ignore = false;
			bool should_ignore = (ignore_pattern.find(nd->name + ",")
			 	!= std::string::npos
			);

			std::string tmp;
			if(should_ignore){
				tmp = "{0}";
			}else{
				tmp = resolve_template(cf, verbosity);
			}
			s = fmt::vformat(std::string_view(tmp), *arg_strs);
			delete arg_strs;

			if(stack.size() == 0){
				keep_looping = false;
				break;
			}

			if(use_shorthand && !should_ignore){
                auto parent_nd = std::get<0>(stack[stack.size()-1])->origin_data;
                auto parent_tmp = use_shorthand ? parent_nd->shorthand_template : parent_nd->expr_template;
                if( tmp[tmp.size()-1] != ')' && 
                 	parent_tmp[parent_tmp.size()-1] != ')'){

                    s = fmt::format("({})", s);
                }
            }

            std::tie(cf, i, arg_strs) = stack[stack.size()-1];
            stack.pop_back();
            arg_strs->push_back(s);
		}
	}
	return s;
}

std::ostream& operator<<(std::ostream& out, Func* func){
	return out << func->to_string();
}


// -------------------------------------------------
// set_arg()


// void Func::set_const_arg(size_t i, const Item& val){
// 	if(i > this->head_infos.size()){
// 		throw std::invalid_argument("Setting Func arg out of range.");
// 	}

// 	if(!val.is_primitive()){
// 		throw std::invalid_argument("Argument must be primitive, got " + val.to_string());
// 	}

// 	this->is_initialized = false;
// 	auto& head_infos = this->head_infos;
// 	size_t start = this->head_ranges[i].start;
// 	size_t end = this->head_ranges[i].end;

// 	for(size_t j = start; j < end; ++j){
// 		auto& head_info = head_infos[j];
// 		Func* cf = head_info.cf_ptr;
// 		auto arg_ind = head_info.arg_ind;

// 		head_info.kind = ARGINFO_CONST;
// 		head_info.has_deref = false;

// 		// TODO: Set the actual value
// 		this->set(j, val);

// 		cf->root_arg_infos[arg_ind].kind = ARGINFO_CONST;
// 		cf->root_arg_infos[arg_ind].has_deref = 0;
// 		cf->root_arg_infos[arg_ind].ptr = head_info.head_data_ptr;
// 		cf->root_arg_infos[arg_ind].t_id = head_info.head_t_id;
// 	}
	
// }

// Set a particular arugment as part of the dynamic composition of a
//  Func. For instance, set_arg() might set a constant, Var, or Func
//	as part of a composition. For instance if we have:
//  add3(a,b,c) = a + b + c, and we do f = add3(1, Var('z', int), add3)
//  this will make a copy of add3 and call: set_arg(0, 1), 
//  set_arg(0, Var('z', int)), set_arg(0, add3), resulting in  
//  f = 1 + z + (a + b+ c)

void Func::set_arg(size_t i, const Item& val){
	if(i > this->head_infos.size()){
		throw std::invalid_argument("Setting Func arg out of range.");
	}
	this->is_initialized = false;
	auto& head_infos = this->head_infos;
	size_t start = this->head_ranges[i].start;
	size_t end = this->head_ranges[i].end;

	uint8_t kind = val.t_id == T_ID_VAR ? ARGINFO_VAR :
				   val.t_id == T_ID_FUNC ? ARGINFO_FUNC_UNEXPANDED :
				   ARGINFO_CONST;

	// if(!kind){
	// 	throw std::invalid_argument(
	// 	"Invalid value provided for Func::set_arg(), but got " + val.to_string() + 
	// 	" with t_id=" + std::to_string(val.t_id) + ". Expected int, float, string, Var or Func.");	
	// }


	
	for(size_t j = start; j < end; ++j){
		auto& head_info = head_infos[j];

		Func* cf = head_info.cf_ptr;
		auto arg_ind = head_info.arg_ind;
		head_info.kind = kind;

		bool has_deref = false;
		switch(kind) {
        	case ARGINFO_VAR:
        	{
        		Var* var = val.as_var();
        		// TODO: Check output Type
        		// if(head_info.base_t_id != var->head_t_id && !this->is_ptr_func){
            	// 	throw std::invalid_argument("Var's head_type doesn't match composing CREFunc's argument type.");
        		// }

            	ref<Var> head_var;
        		if(head_info.has_deref){
		            auto old_head_var = head_info.var_ptr;
		            // head_var = var_extend(var, old_head_var.deref_infos)
		            has_deref = true;
		        }else{
		            head_var = var;
		            has_deref = var->size() > 0;
		        }

		        head_info.has_deref = has_deref;
		        head_info.var_ptr = head_var.get();
		    }
        		break;
        	case ARGINFO_FUNC_UNEXPANDED:
        		// TODO check func return type

        		head_info.cf_ptr = val.as_func();
        		break;
        	// default:
        		// Do nothing
       	}
		head_info.has_deref = false;

		this->set(j, val);

		cf->root_arg_infos[arg_ind].kind = kind;
		cf->root_arg_infos[arg_ind].has_deref = has_deref;
		cf->root_arg_infos[arg_ind].ptr = head_info.head_data_ptr;
		cf->root_arg_infos[arg_ind].type = head_info.head_type;
	}
	
}


// ---------------------------------------------------------------
// reinitialize()

void Func::reinitialize(){

	using base_heads_pair_t = std::tuple<void*,std::vector<HeadInfo>>;
	if(this->is_initialized) return;

	uint16_t max_arg_depth = 0;


	// Build new base_var_map. This dictates a new signature when there are  
	//  repeated Vars in a Func's expression. For instance, f = add(A, A),
	//  maps a=Var("A", int) to two args for an effective signature of f(A). 
	std::map<void*, size_t> base_var_map = {};
	std::vector<base_heads_pair_t> base_vars = {};
	size_t n_vars = 0;

	for(auto hrng : this->head_ranges){
		for(uint16_t j=hrng.start; j < hrng.end; ++j){
			HeadInfo& head_info = this->head_infos[j];

			// For ARGINFO_VAR kinds insert base_ptr into the base_var_map
			if(head_info.kind == ARGINFO_VAR){
				Var* var = head_info.var_ptr;
				void* base_ptr = (void*) var->base;

				auto [it, inserted] = base_var_map.try_emplace(
					base_ptr, n_vars);
				if(inserted){
					std::vector<HeadInfo> var_head_info = {};
					base_vars.push_back({base_ptr, var_head_info});
					++n_vars;
				}

				std::get<1>(base_vars[it->second]).push_back(head_info);

			// For ARGINFO_FUNC_UNEXPANDED kind insert the base_ptrs of all of the
            //  CRE_Funcs's base vars into base_var_map
			}else if(head_info.kind == ARGINFO_FUNC_UNEXPANDED){
				Func* cf = head_info.cf_ptr;

				for(auto& hrng_k : cf->head_ranges){
					for(uint16_t n=hrng.start; n < hrng.end; ++n){
						HeadInfo& head_info_n = cf->head_infos[n];

						Var* var = head_info_n.var_ptr;
						void* base_ptr = (void*) var->base;

						// std::vector<HeadInfo> var_head_info = {};
						auto [it, inserted] = base_var_map.try_emplace(
							base_ptr, n_vars);
						if(inserted){
							std::vector<HeadInfo> var_head_info = {};
							base_vars.push_back({base_ptr, var_head_info});
							++n_vars;
						}
						std::get<1>(base_vars[it->second]).push_back(head_info);
						// it->second.push_back(head_info_n);
					}
					max_arg_depth = std::max(cf->depth, max_arg_depth);
				}
			}
		}
	}

	// Make new head_ranges according to base_var_map
	size_t n_bases = base_var_map.size();
	std::vector<HeadRange> head_ranges;
	head_ranges.reserve(n_bases);
	size_t n_heads = 0;
	for(auto [_, base_head_infos] : base_vars){
		cout << "SIZE:" << base_head_infos.size() << endl;
		head_ranges.emplace_back(n_heads, n_heads+base_head_infos.size());
		n_heads += base_head_infos.size();
	}


	// Make new head_infos according to base_var_map
	std::vector<HeadInfo> head_infos;
	head_infos.reserve(n_heads);
	for(auto [base_ptr, base_head_infos] : base_vars){
		Var* base_var = (Var *) base_ptr;
		for(HeadInfo& base_head_info : base_head_infos){
			HeadInfo hi = base_head_info;
			hi.base_type = base_var->base_type;

			head_infos.push_back(std::move(hi));
		}
	}

	this->n_args = n_bases;

	// build_instr_set

	// Record if the CREFunc was modified enough from it's base definition
    //  that it should be considered 'is_composed'. Also check 'has_any_derefs'.
    bool any_hd = false;
    bool is_composed = false;
    for(auto inf : this->root_arg_infos){
    	any_hd = any_hd | inf.has_deref;
    	if(any_hd || inf.kind == ARGINFO_FUNC || inf.kind == ARGINFO_CONST){
            is_composed = true;
    	}
    }
    this->has_any_derefs = any_hd;
    this->is_composed = is_composed;
    this->depth = this->depth + max_arg_depth;
}
