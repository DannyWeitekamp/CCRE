#include <cstddef>
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
#include <limits>
#include "../include/intern.h"
#include "../include/func.h"
#include "../include/types.h"
#include "../include/var.h"
#include "../include/alloc_buffer.h"
#include "../include/builtin_funcs.h"
#include "../include/literal.h"


namespace cre {

void Func_dtor(const CRE_Obj* x){

	Func* func = (Func*) x;

	// cout << "FUNC_DTOR: " << func << endl;

	// Release all of function's members
	for(size_t i=0; i < func->root_arg_infos.size(); ++i){
		func->get(i)->release();
	}

	// Explicitly delete the bytecode
	if(func->bytecode != nullptr){
		delete[] func->bytecode;
	}

	if(func->is_origin){
		delete func->origin_data;	
	}
	
	// Explicitly call the builtin destructor to clear various std::vector
	func->~Func();
	CRE_Obj_dtor(x);
}

std::string make_default_template(
	std::string_view name, size_t n_args){

	std::vector<std::string> brackets(n_args, "{}");
	return fmt::format("{}({})", name, fmt::join(brackets, ", "));
}


//------------------------------------------------------------
// new_func()

// Func* alloc_func(uint32_t length, AllocBuffer* buffer=nullptr){
  

//   return (Func*) func_addr;


ref<Func> new_func(StackCallFunc stack_call_func,
				   StackCallFunc2 stack_call_func2,
				   PtrToItemFunc ptr_to_item_func,
				   CallRecursiveFunc call_recursive_fc,
				   size_t n_args, 
				   OriginData* origin_data,
				   AllocBuffer* buffer){
	auto [func_addr, did_malloc] =  alloc_cre_obj(SIZEOF_FUNC(n_args), &Func_dtor, T_ID_FUNC, buffer);
	// Func* func = alloc_func(n_args);
	ref<Func> func = new (func_addr) Func(
			stack_call_func, stack_call_func2, ptr_to_item_func,
			 call_recursive_fc, n_args, origin_data);
	
	return func;
}

// ----------------------------------------------------------
//  define_func()

void _init_arg_specs(Func* func, 
					const std::vector<CRE_Type*>& arg_types,
					const std::vector<uint16_t>& offsets){
	size_t n_args = arg_types.size();

	std::vector<ArgInfo>& root_arg_infos = func->root_arg_infos;
	root_arg_infos.reserve(n_args);

 	std::vector<HeadInfo>& head_infos = func->head_infos;
	head_infos.reserve(n_args);

	std::vector<HeadRange>& head_ranges = func->head_ranges;
	head_ranges.reserve(n_args);

	// for(auto arg_typqe : arg_types){
	uint16_t offset = 0;
	for(uint32_t i =0; i< arg_types.size(); ++i){
		CRE_Type* arg_type = arg_types[i];

		auto alias = intern(fmt::format("x{}", i));
		ref<Var> var = new_var(alias, arg_type);

		// cout << "-REFCOUNT V: " << var->get_refcount() << endl;
		func->set(i, Item(var));
		// cout << "+REFCOUNT V: " << var->get_refcount() << endl;

		// ArgInfo
		// uint16_t t_id = arg_type->t_id;
		root_arg_infos.emplace_back(
			arg_type, ARGINFO_VAR, offsets[i], i, i, false);
		// offset += arg_type->byte_width;

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
		// hi.arg_data_ptr = nullptr;
		// hi.head_data_ptr = nullptr;
		// hi.byte_width = 0;
		hi.ref_kind = 0; // Necessary ? 

		head_infos.push_back(hi);

		// Head Range
		head_ranges.emplace_back(i, i+1);
	}
 }

FuncRef define_func(
		std::string_view name, 
		StackCallFunc cfunc_ptr,
		StackCallFunc2 cfunc_ptr2,
		PtrToItemFunc ptr_to_item_func,
		CallRecursiveFunc call_recursive_fc,
		size_t stack_size,
		const std::vector<uint16_t>& offsets,
		CRE_Type* ret_type,
		const std::vector<CRE_Type*>& arg_types,
		std::string_view expr_template,
		std::string_view shorthand_template,
		std::string_view negated_shorthand_template){

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

	if(negated_shorthand_template.size() == 0){
		if(shorthand_template.size() == 0){
			origin_data->negated_shorthand_template = fmt::format("~{}", origin_data->expr_template);
		}else{
			origin_data->negated_shorthand_template = fmt::format("~({})", shorthand_template);
		}
	}else{
		origin_data->negated_shorthand_template = negated_shorthand_template;
	}

	ref<Func> func = new_func(
		cfunc_ptr, cfunc_ptr2, ptr_to_item_func,
		call_recursive_fc, n_args, origin_data);//new Func(origin_data, cfunc_ptr);
	// func->init_control_block(&Func_dtor);
	// func->dtor = &Func_dtor;

	origin_data->func = func.get();

	_init_arg_specs(func, arg_types, offsets);
	func->return_type = ret_type;
	func->return_stack_offset = offsets[arg_types.size()];
	func->is_origin = true;
	func->stack_size = stack_size;
	// cout << "ST@@CK: " << stack_size << endl;

	func->reinitialize();

	// cout << "DEFINE FUNC: " << uint64_t(func.get()) << endl;

	return (FuncRef) func;
}

// -------------------------------------------------------------
// copy()
FuncRef Func::copy_shallow(AllocBuffer* alloc_buffer){
	ref<Func> nf = new_func(
		stack_call_func, stack_call_func2, ptr_to_item_func,
		call_recursive_fc, n_root_args, origin_data, alloc_buffer);

	// nf->dtor = &Func_dtor;

	nf->return_type = return_type;
	nf->n_args = n_args;
	nf->n_root_args = n_root_args;

	nf->root_arg_infos = root_arg_infos;
	nf->head_ranges = head_ranges;
	nf->head_infos = head_infos;
	nf->stack_size = stack_size;
	nf->return_stack_offset = return_stack_offset;

	// Make the arg_data_ptr and head_data_ptr point to the copy
	for(size_t i=0; i < head_infos.size(); ++i){
		HeadInfo& head_info = head_infos[i];
		nf->head_infos[i] = head_info;
		if(head_info.cf_ptr == this){
			nf->head_infos[i].cf_ptr = nf.get();
			// nf->head_infos[i].arg_data_ptr = ??;
			// nf->head_infos[i].head_data_ptr = ??;
		}
	}

	for(size_t i=0; i < root_arg_infos.size(); ++i){
		nf->set_init(i, *this->get(i));
		// Item* mbr_ptr = nf->get(i);
		// nf->root_arg_infos[i].ptr = mbr_ptr;
	}

	// cout << "-:" << uint64_t(this) << "," << uint64_t(root_arg_infos[0].ptr) << endl;
	// cout << "+:" << uint64_t(nf.get()) << "," << uint64_t(nf->root_arg_infos[0].ptr) << endl;

	nf->is_initialized = false;	
	nf->is_ptr_func = is_ptr_func;	
	nf->depth = depth;	
	nf->has_any_derefs = has_any_derefs;	
	nf->is_composed = is_composed;	
	return (FuncRef) nf;
}

FuncRef Func::copy_deep(AllocBuffer* alloc_buffer){


	ref<Func> cpy;
	if(depth <= 1){
		cpy = copy_shallow(alloc_buffer);
		// cout << "SHALLOW COPY: " << uint64_t(this) << ", " << uint64_t(cpy.get()) << endl;
		return (FuncRef) cpy;
	}

	using stack_tuple = std::tuple<Func*, int, std::vector<ref<Func>>*>  ;
	std::vector<stack_tuple> stack = {};
	size_t i = 0;
	Func* cf = this;
	std::vector<ref<Func>>* func_copies = new std::vector<ref<Func>>;

	// cout << "--START DEEP--" << endl;
	std::map<Func*, Func*> remap = {};

	bool keep_looping = true;
	
	while(keep_looping){
		// cout << "LOOP: " << stack.size() << ", " << i << endl;

		ArgInfo arg_info = cf->root_arg_infos[i];
		if(arg_info.kind == ARGINFO_FUNC){
			stack.push_back(stack_tuple(cf, i+1, func_copies));
			cf = (Func*) cf->get(i)->_as<Func*>();
			func_copies = new std::vector<ref<Func>>;
			i =0;
		}else{
			i += 1;
		}
	

		// When past end arg of deepest func, copy it. Then pop prev frame from stack.
		
		while(i >= cf->root_arg_infos.size()){
	        cpy = cf->copy_shallow(alloc_buffer);
	    	size_t j = 0;
	    	for(size_t k=0; k < cpy->root_arg_infos.size(); ++k){
	    		ArgInfo& inf = cpy->root_arg_infos[k];
	    		if(inf.kind == ARGINFO_FUNC){
	    			ref<Func> sub_func = (*func_copies)[j];
	    			// cout << "-REFCOUNT: " << sub_func->get_refcount() << endl;
	    			cpy->set(k, Item(sub_func));
	    			// cout << "+REFCOUNT: " << sub_func->get_refcount() << endl;
	    			// inf.ptr = cpy->get(k); 
	    			j += 1;
	    		}//else{
	    			// cpy->set(k, *cf->get(k));
	    		// }
	    	}

	    	remap[cf] = cpy;
	    	for(size_t k=0; k < cpy->head_infos.size(); ++k){
	    		cpy->head_infos[k].cf_ptr = remap[cf->head_infos[k].cf_ptr];
	    	}

	    	// remap[cf] = cpy;
	    	// remap[return data ptr]

	    	// Remap head info data ptrs
	    	// for(size_t k=0; k < cpy->head_infos.size()){
	    	// 	HeadInfo hi = cf->head_infos[k];
	    	// 	HeadInfo hi_cpy = cpy->head_infos[k];
	    	// 	// remap data ptrs
	    	// }

	    	// Pointers in bytecode (probabably don need)
	    	// ...

	    	
	    	// cout << "MOO: " << func_copies->size() << endl;
	    	delete func_copies;

	    	// End Case: Stack Exhausted

	    	if(stack.size() == 0){
	    		keep_looping = false;
	    		break;
	    	}

	    	std::tie(cf, i, func_copies) = stack[stack.size()-1];
	        stack.pop_back();
	        func_copies->push_back(cpy);
	    }
    }
    
    // cout << "DEEP COPY: " << uint64_t(this) << ", " << uint64_t(cpy.get()) << endl;
    return (FuncRef) cpy;
}


// -------------------------------------------------------------
// to_string()

std::string resolve_template(Func* func, uint8_t verbosity, bool negated){
	OriginData* od = func->origin_data;
	bool use_shorthand = (verbosity <= DEFAULT_VERBOSITY) && (od->shorthand_template.size() > 0);
	if(use_shorthand && !negated && od->shorthand_template.size() > 0){
		return od->shorthand_template;
	}else if(use_shorthand && negated && od->negated_shorthand_template.size() > 0){
		return od->negated_shorthand_template;
	}else if(od->expr_template.size() > 0){
		return od->expr_template;
	}else{
		throw std::runtime_error("Func has no template.");
	}
}




std::string Func::to_string(uint8_t verbosity, bool negated){
	using fmt_args_t = fmt::dynamic_format_arg_store<fmt::format_context>;
	using stack_tuple = std::tuple<Func*, int, fmt_args_t*> ;

	// cout << "TO STRING: " << uint64_t(this) << endl;

	OriginData* od = this->origin_data;
	bool use_derefs = true;
	bool use_shorthand = (verbosity <= DEFAULT_VERBOSITY) && (od->shorthand_template.size() > 0);

	std::string ignore_pattern = "";

	std::vector<stack_tuple> stack = {};
	Func* cf = (Func*) this;
	int i = 0;
	auto arg_strs = new fmt_args_t();
	std::string s;
	bool keep_looping = true;	
	while(keep_looping){
		// cout << "S LOOP: " << stack.size() << ", " << i << endl;
		// cout << "RARI:" << root_arg_infos.size() << endl; 
		if(i < cf->root_arg_infos.size()){
			auto& arg_info = cf->root_arg_infos[i];

			// cout << ">> i=" << i << ", kind=" << arg_info.kind << endl;

			if(arg_info.kind == ARGINFO_FUNC){
				// cout << "FUNC0 :" << uint64_t(cf) << endl;
				
				// Recurse Case: Func

				stack.emplace_back(stack_tuple(cf, i+1, arg_strs));

				// cf = cf->get(i).as_func(); 
				cf = (*cf->get(i))._as<Func*>();
				// cout << "FUNC  :" << uint64_t(cf) << endl;
				arg_strs = new fmt_args_t();
				i =0;
			}else{
				// Terminal Case: Var/Const
				if(arg_info.kind == ARGINFO_VAR){
					// cout << "VAR:" << uint64_t(cf->get(i)) << endl;
					Var* var =  (*cf->get(i))._as<Var*>();
					// Var* var = (*arg_info.ptr).as_var();
					// cout << "VAR:" << uint64_t(arg_info.ptr) << endl;

					// cout << "+VAR: " << var->get_alias_str() << endl;
					
					arg_strs->push_back(var->to_string());
					
				}else if(arg_info.kind == ARGINFO_CONST){
					// cout << "CONST:" << uint64_t(arg_info.ptr) << endl;
					Item& item = *cf->get(i);
					arg_strs->push_back(item.to_string());
				}else{
					throw std::runtime_error("Bad root_arg_infos[" + std::to_string(i) + "].kind: " + std::to_string(arg_info.kind));
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
				tmp = resolve_template(cf, verbosity, negated);
			}
			s = fmt::vformat(std::string_view(tmp), *arg_strs);
			delete arg_strs;

			if(stack.size() == 0){
				keep_looping = false;
				break;
			}


			if(use_shorthand && !should_ignore){
                auto parent_nd = std::get<0>(stack[stack.size()-1])->origin_data;
				std::string parent_tmp;
				if(negated && parent_nd->negated_shorthand_template.size() > 0){
					parent_tmp = parent_nd->negated_shorthand_template;
				}else{
					parent_tmp = use_shorthand ? parent_nd->shorthand_template : parent_nd->expr_template;
				}
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
	// cout << endl;
	return s;
}

std::ostream& operator<<(std::ostream& out, Func* func){
	return out << func->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Func> func){
	return out << func.get()->to_string();
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
// 		cf->root_arg_infos[arg_ind].get_t_id() = head_info.head_t_id;
// 	}
	
// }



// Checks if an 
std::tuple<bool, Func*> check_cast_arg(uint16_t val_t_id, CRE_Type* target_type){
	if(target_type->get_t_id() == val_t_id){
		return {true, nullptr};
	}

	switch(target_type->get_t_id()) {
		case T_ID_BOOL:
			if(t_id_is_numerical(val_t_id)){
				// cast to bool
			}else if(val_t_id == T_ID_STR){
				// cast from string
			}
			break;
    	case T_ID_INT:
			if(t_id_is_numerical(val_t_id)){
				// cast to int
			}else if(val_t_id == T_ID_STR){
				// cast from string
			}
    		break;
    	case T_ID_FLOAT:
    		if(t_id_is_numerical(val_t_id)){
				// cast to double	
			}else if(val_t_id == T_ID_STR){
				// cast from string
			}
    		break;
    	case T_ID_STR:
    	{
    		if(val_t_id == T_ID_BOOL){
    			// bool to str
    		} else if(val_t_id == T_ID_INT){
    			// int to str
    		} else if(val_t_id == T_ID_FLOAT){
    			// double to str
    		}
    		break;
    	}
  }
  return {true, nullptr};
}



bool cast_allowed(uint16_t src_t_id, uint16_t t_id){
	switch(src_t_id){

	case T_ID_UNDEF:
		return false;

	case T_ID_NONE:
		return any_of(t_id, T_ID_NONE, T_ID_BOOL, T_ID_STR);

	case T_ID_BOOL:
		return any_of(t_id, T_ID_BOOL, T_ID_INT, T_ID_FLOAT, T_ID_STR);

	case T_ID_INT:
		return any_of(t_id, T_ID_BOOL, T_ID_INT, T_ID_FLOAT, T_ID_STR);

	case T_ID_FLOAT:
		return any_of(t_id, T_ID_BOOL, T_ID_INT, T_ID_FLOAT, T_ID_STR);

	case T_ID_PTR:
		return any_of(t_id, T_ID_OBJ);

	case T_ID_STR:{ 
		return any_of(t_id, T_ID_STR, T_ID_NONE, T_ID_FLOAT, T_ID_INT, T_ID_BOOL);
	}

	// Rest handles objects
	default:
		if(any_of(t_id, src_t_id, T_ID_BOOL)){
			return true;
		}else if(t_id_is_obj(t_id) and any_of(t_id, T_ID_OBJ)){
			return true;	
		}
		
	}
	return false;
}

bool cast_allowed(CRE_Type* src, CRE_Type* target){
	uint16_t src_t_id = src->get_t_id();
	uint16_t t_id = target->get_t_id();
	return cast_allowed(src_t_id, t_id);
}




// Set a particular argument as part of the dynamic composition of a
//  Func. For instance, set_arg() might set a constant, Var, or Func
//	as part of a composition. For instance if we have:
//  add3(a,b,c) = a + b + c, and we do f = add3(1, Var('z', int), add3)
//  this will make a copy of add3 and call: set_arg(0, 1), 
//  set_arg(1, Var('z', int)), set_arg(2, add3), resulting in  
//  f = 1 + z + (a + b+ c)

void Func::set_arg(size_t i, const Item& val){
	// cout << "SIZE" << this->head_ranges.size() << endl;

	if(i >= this->head_ranges.size()){
		throw std::invalid_argument("Setting Func arg out of range.");
	}

	

	this->is_initialized = false;
	// auto& head_infos = this->head_infos;
	size_t start = this->head_ranges[i].start;
	size_t end = this->head_ranges[i].end;
	
	uint8_t kind = val.get_t_id() == T_ID_VAR ? ARGINFO_VAR :
				   val.get_t_id() == T_ID_FUNC ? ARGINFO_FUNC :
				   val.get_t_id() == T_ID_LITERAL ? ARGINFO_FUNC :
				   ARGINFO_CONST;

	// cout << uint64_t(this) << "  set_arg() i=" << i << ", val=" << val << ", t_id=" << uint64_t(val.get_t_id()) << ", kind=" << uint64_t(kind) << endl;

	// if(!kind){
	// 	throw std::invalid_argument(
	// 	"Invalid value provided for Func::set_arg(), but got " + val.to_string() + 
	// 	" with t_id=" + std::to_string(val.get_t_id()) + ". Expected int, float, string, Var or Func.");	
	// }

	// cout << "SET_ARG: " << this->to_string() << ", i=" << i << ", val=" << val.to_string() << ", t_id=" << val.get_t_id() << ", kind=" << uint64_t(kind) << endl;

	
	for(size_t j = start; j < end; ++j){


		HeadInfo& head_info = head_infos[j];
 
		Func* cf = head_info.cf_ptr;
		

		auto arg_ind = head_info.arg_ind;
		head_info.kind = kind;

		// cout << "  CF:" << cf << "; arg_ind=" << arg_ind << endl;

		

		bool has_deref = false;
		switch(kind) {
        	case ARGINFO_VAR:
        	{
        		Var* var = val._as<Var*>();
        		// TODO: Check output Type
        		// if(head_info.base_t_id != var->head_t_id && !this->is_ptr_func){
            	// 	throw std::invalid_argument("Var's head_type doesn't match composing CREFunc's argument type.");
        		// }
				// cout << "VAR: " << var->to_string() << ", head_type=" << head_info.base_type->to_string() << ", var->head_type=" << var->head_type->to_string() << endl;
        		if(var->head_type->get_t_id() != T_ID_UNDEF &&
				   !cast_allowed(var->head_type, head_info.base_type)){
				   cout << "BAD ARG: " << var->to_string() << ", " << head_info.base_type->to_string() << ", " << var->head_type->to_string() << endl;
        		   throw_bad_arg(i, var, head_info.base_type, var->head_type, false);
        		}

            	ref<Var> head_var;
        		if(head_info.has_deref){
        			
		            auto old_head_var = head_info.var_ptr;

		            // cout << "*THIS CASE: " << var << ", " << old_head_var << endl;
		            // cout << "+THIS CASE: " << var->length << ", " << old_head_var->length << endl;
		            // TODO;
		            head_var = var->_extend_unsafe(old_head_var->deref_infos, old_head_var->length);//var_extend(var, old_head_var.deref_infos)
		            // cout << "-THIS CASE: " << head_var << ", " << head_var->length << endl;
		            has_deref = true;
		            cf->set(arg_ind, Item(head_var));
		        }else{
		            head_var = var;
		            has_deref = var->size() > 0;
		            cf->set(arg_ind, val.to_strong());
		        }

		        head_info.has_deref = has_deref;
		        head_info.var_ptr = head_var.get();
		    }
        		break;
        	case ARGINFO_FUNC:
        	{
				Func* func = nullptr;
				if(val.get_t_id() == T_ID_LITERAL){
					Literal* lit = val._as<Literal*>();
					if(!lit->is_func()){
						throw std::invalid_argument(fmt::format("Cannot compose non-Func literal: {}", lit->to_string()));
					}
					func = (Func*) lit->obj.get();
					if(lit->negated){
						func = NotFunc->compose(func);
					}
				}else{
					func = val._as<Func*>();
				}
        		
        		if(not cast_allowed(func->return_type, head_info.base_type)){
        			throw_bad_arg(i, func, head_info.base_type, func->return_type, false);
        		}
        		// TODO check func return type
        		head_info.kind = ARGINFO_FUNC_UNEXPANDED;
        		head_info.cf_ptr = func;
        		cf->set(arg_ind, val);
        		break;
        	}
        	default:
        		CRE_Type* val_type = val.get_type();

        		if(not cast_allowed(val_type, head_info.base_type)){
        			throw_bad_arg(i, val, head_info.base_type, val_type, false);
        		}

        		cf->set(arg_ind, val);
        		// Do nothing
       	}
		head_info.has_deref = has_deref;

		// cout << "set j=" << j << ", arg_ind=" << arg_ind << endl;

		cf->root_arg_infos[arg_ind].kind = kind;
		cf->root_arg_infos[arg_ind].has_deref = has_deref;
		// cf->root_arg_infos[arg_ind].ptr = cf->get(arg_ind);
		cf->root_arg_infos[arg_ind].type = head_info.head_type;
	}
	
}





// ---------------------------------------------------------------
// Bytecode Generation

// Execution Stack Has: [...?b arg_vals] [...?b intermediates] [...?b return]

// Bytecode format:
const uint16_t BC_READ_CONST = 1;
const uint16_t BC_DEREF_VAR = 2;
const uint16_t BC_CALL_FUNC = 3;
const uint16_t BC_CLEANUP = 4;

// ----------------
// : READ_CONST
// [2b kind] [2 next_offset] [2b byte-width] [2b dest_offset] [...?b data] 
size_t sizeof_load_const(){
	return 8;
}

void write_load_const(uint8_t* bytecode, uint16_t src_index, uint16_t dest_offset){
	uint16_t* rc_instr = (uint16_t*) bytecode;
	rc_instr[0] = BC_READ_CONST;
	rc_instr[1] = sizeof_load_const();
	rc_instr[2] = src_index;
	rc_instr[3] = dest_offset;
}

// ----------------
// : DEREF_VAR
// [2b kind] [2 next_offset] [2b src_offset] [2b dest_offset] 
// [..16b DerefInfos]

size_t sizeof_deref_var(Var* var){
	return 8 + sizeof(DerefInfo) * var->size();
}

void write_deref_var(uint8_t* bytecode, Var* var, uint16_t src_offset, uint16_t dest_offset){
	uint16_t* rc_instr = (uint16_t*) bytecode;
	rc_instr[0] = BC_DEREF_VAR;
	rc_instr[1] = sizeof_deref_var(var);
	rc_instr[2] = src_offset;
	rc_instr[3] = dest_offset;
	DerefInfo* df_info = (DerefInfo*) &rc_instr[4];
	memcpy(df_info, var->deref_infos, sizeof(DerefInfo) * var->size());
}

// ----------------
// : CALL_FUNC
// [2b kind] [2 next_offset] [2b ret_offset] [2b n_args] [8b func*] 
//    [...2b arg_offset] [?b pad] 
size_t sizeof_call_func(Func* cf){
	return 8 + 8 + 2 * (cf->n_root_args + cf->n_root_args % 4);
}

void write_call_func(uint8_t* bytecode, Func* cf, uint16_t ret_offset, uint16_t* head_offsets){

	// cout << "Bytecode start:" << uint64_t(bytecode) << endl;
	uint16_t* rc_instr = (uint16_t*) bytecode;
	rc_instr[0] = BC_CALL_FUNC;
	rc_instr[1] = sizeof_call_func(cf);
	rc_instr[2] = ret_offset;
	rc_instr[3] = cf->n_root_args;
	// cout << "<::" << uint64_t(cf) << endl;
	Func** func_ptr = (Func**) &rc_instr[4];
	*func_ptr = cf;
	


	uint16_t* self_arg_offsets = (uint16_t*) &rc_instr[8];

	// cout << "<::" << uint64_t(self_arg_offsets)-uint64_t(bytecode) << " / " << rc_instr[1] << endl;
	for(uint16_t i=0; i < cf->n_root_args; i++){
		// TODO
		// cout << "<::" << uint64_t(&self_arg_offsets[i])-uint64_t(bytecode) << " / " << rc_instr[1] << ", " << uint64_t(&self_arg_offsets[i]) << endl;
		self_arg_offsets[i] = head_offsets[i];
	}
}

// ----------------
// : CLEANUP - [Free Str or Decref]
// [2b kind] [2 next_offset] [4b pad] [8b f_ptr*] 
typedef void(*cleanup_t)(uint8_t* data);

size_t sizeof_cleanup(){
	return 16;
}

void write_cleanup(uint8_t* bytecode, cleanup_t f_ptr){
	uint16_t* rc_instr = (uint16_t*) bytecode;
	rc_instr[0] = BC_CLEANUP;
	rc_instr[1] = sizeof_cleanup();
	rc_instr[2] = 0;
	rc_instr[3] = 0;

	cleanup_t* self_f_ptr = (cleanup_t*) &rc_instr[8];
	self_f_ptr[0] = f_ptr;
}


// size_t Func::calc_byte_code_size(){
// 	// First pass to find the size of the bytecode
// 	size_t bytecode_len = 0;
// 	for(size_t i=0; i < this->root_arg_infos.size(); ++i){
// 		auto arg_info = this->root_arg_infos[i];
// 		switch(arg_info.kind){
// 			case ARGINFO_CONST:
// 			{
// 				bytecode_len += sizeof_load_const();
// 				break;
// 			}
// 			case ARGINFO_VAR:
// 			{
// 				Var* var = this->get(i)->as_var();
// 				// cout << "#" << uint64_t(var) << endl;
// 				if(var->size() > 0){
// 					bytecode_len += sizeof_deref_var(var);	
// 				}
// 				break;
// 			}
// 			case ARGINFO_FUNC_UNEXPANDED:
// 			{
// 				Func* cf = this->get(i)->as_func();
// 				bytecode_len += sizeof_call_func(cf);	
// 				break;
// 			}
// 		}
// 	}
// 	bytecode_len += sizeof_call_func(this);	
// 	return bytecode_len;
// }

std::string Func::bytecode_to_string() {
	uint8_t* bc = bytecode;
	std::stringstream ss;
		
	// cout << "LEN: " << bytecode_end-bc << endl;
	while(bc < bytecode_end){
		uint16_t* rc_instr = (uint16_t*) bc;	
		switch(rc_instr[0]){
			case BC_READ_CONST:
				ss << fmt::format("CONST: [{}]->@{}\n", rc_instr[2], rc_instr[3]);
				break;
			case BC_DEREF_VAR:
				ss << fmt::format("DEREF: @{}->@{}\n", rc_instr[2], rc_instr[3]);
				break;
			case BC_CALL_FUNC:
			{
				std::vector<std::string> arg_strs = {};
				uint16_t n_args = rc_instr[3];
				Func* cf = *((Func**) &rc_instr[4]);
				// cout << "::" << uint64_t(cf) << endl;
				for(int i=0; i < n_args; ++i){ 
					arg_strs.push_back(fmt::format("@{}", rc_instr[8+i]));
				}
				
				ss << fmt::format("CALL : {}({})->@{}\n", cf->origin_data->name, fmt::join(arg_strs, ", "), rc_instr[2]);
				break;
			}
		}
		// cout << rc_instr[0] << " BC: " << uint64_t(bc) << ", " << rc_instr[1] << endl;
		if(rc_instr[1] == 0){
			break;
		}
		bc += rc_instr[1];
	}
	return ss.str();
}


uint16_t _calc_bytecode_length(Func* cf){
	uint16_t* arg_offsets = (uint16_t*) alloca(cf->n_root_args * sizeof(uint16_t));
	uint32_t bytecode_len = 0;

	for(size_t i=0; i < cf->root_arg_infos.size(); ++i){
		auto& arg_info = cf->root_arg_infos[i];
		switch(arg_info.kind){

		case ARGINFO_FUNC:
		{
			Func* func =  (*cf->get(i))._as<Func*>();
			bytecode_len += _calc_bytecode_length(func);
			break;
		}
		case ARGINFO_CONST:
			bytecode_len += sizeof_load_const();
			break;
		case ARGINFO_VAR:
		{
			Var* var =  (*cf->get(i))._as<Var*>();
			if(var->size() > 0){
				bytecode_len += sizeof_deref_var(var);
			}
			break;
		}
		default:
			throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
		}
	}
	bytecode_len += sizeof_call_func(cf);
	return bytecode_len;
}

uint16_t Func::calc_bytecode_length(){
	uint32_t bytecode_len = _calc_bytecode_length(this);
	if(bytecode_len > std::numeric_limits<uint16_t>::max()){
		throw std::runtime_error("CREFunc Bytecode exceeds std::numeric_limits<uint16_t>::max()=" + std::to_string(std::numeric_limits<uint16_t>::max()));
	}
	return (uint16_t) bytecode_len;
}
	// write_call_func(bc_head, cf, stack_offset, arg_offsets);
	// bc_head += sizeof_call_func(cf);
	
	// return {bc_head, stack_offset};



	// uint16_t bytecode_len = 0;
	// // uint16_t stack_length = 0;

	// using stack_tuple = std::tuple<Func*, int> ;
	// std::vector<stack_tuple> stack = {};
	// Func* cf = (Func*) this;
	// int i = 0;	
	// bool keep_looping = true;	
	// while(keep_looping){
	// 	if(i < cf->root_arg_infos.size()){
	// 		auto& arg_info = cf->root_arg_infos[i];
	// 		if(arg_info.kind == ARGINFO_FUNC){
	// 			// Push stack
	// 			stack.emplace_back(stack_tuple(cf, i+1));
	// 			cf = (*cf->get(i)).as_func();
	// 			i = 0;
	// 		}else{
	// 		 switch(arg_info.kind){
	// 			case ARGINFO_CONST:
	// 				bytecode_len += sizeof_load_const();
	// 				// stack_length += arg_info.type->byte_width;
	// 				break;
	// 			case ARGINFO_VAR:
	// 			{
	// 				Var* var =  (*cf->get(i)).as_var();
	// 				if(var->size() > 0){
	// 					bytecode_len += sizeof_deref_var(var);	
	// 				}
	// 				break;
	// 			}
	// 			default:
	// 				throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
	// 		}}
	// 		++i;
	// 	}

	// 	while(i >= cf->root_arg_infos.size()){
	// 		bytecode_len += sizeof_call_func(cf);
	// 		// stack_length += cf->return_type->byte_width;

	// 		// Check for end case
	// 		if(stack.size() == 0){
	// 			keep_looping = false;
	// 			break;
	// 		}
			
	// 		// Pop stack
    //         std::tie(cf, i) = stack[stack.size()-1];
    //         stack.pop_back();
	// 	}
	// }
	// cout << endl;

// 	return bytecode_len;//, stack_length};
// }


std::tuple<uint8_t*, uint16_t> _write_bytecode(
	Func* cf,
	uint8_t* bc_head, uint16_t stack_offset,
	const std::map<SemanticVarPtr, size_t>& base_var_map,
	const std::vector<uint16_t>& arg_stack_offsets//,
	// uint16_t head_ind
	){

	uint16_t* arg_offsets = (uint16_t*) alloca(cf->n_root_args * sizeof(uint16_t));



	for(size_t i=0; i < cf->root_arg_infos.size(); ++i){
		auto& arg_info = cf->root_arg_infos[i];
		// arg_info.head_ind = head_ind;

		switch(arg_info.kind){

		case ARGINFO_FUNC:
		{
			Func* func =  (*cf->get(i))._as<Func*>();

			
			// Recurse... should reserve offsets 
			// std::tie(bc_head, stack_offset, head_ind) = _write_bytecode(
			std::tie(bc_head, stack_offset) = _write_bytecode(
				func, bc_head, stack_offset,
				base_var_map, arg_stack_offsets//, head_ind
			);
			// Reserve room on stack for return  
			arg_offsets[i] = stack_offset;
			stack_offset += func->return_type->byte_width;

			
			break;
		}
		case ARGINFO_CONST:
			write_load_const(bc_head, i, stack_offset);

			arg_offsets[i] = stack_offset;
			bc_head += sizeof_load_const();
			stack_offset += arg_info.type->byte_width;
			break;
		case ARGINFO_VAR:
		{
			Var* var =  (*cf->get(i))._as<Var*>();
			SemanticVarPtr base_var =  SemanticVarPtr(var->base);

			auto it = base_var_map.find(base_var);
			if(it == base_var_map.end()){
				throw std::runtime_error("Var not found.");
			}
			uint16_t arg_offset = arg_stack_offsets[it->second];

			if(var->size() > 0){
				write_deref_var(bc_head, var, arg_offset, stack_offset);

				arg_offsets[i] = stack_offset;
				bc_head += sizeof_load_const();
				stack_offset += arg_info.type->byte_width;
			}else{
				arg_offsets[i] = arg_offset;
			}

			// ++head_ind;
			break;
		}
		default:
			throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
		}
	}

	write_call_func(bc_head, cf, stack_offset, arg_offsets);
	bc_head += sizeof_call_func(cf);

	// cf->return_stack_offset = stack_offset;
	stack_offset += cf->return_type->byte_width;

	return {bc_head, stack_offset};
}

void Func::write_bytecode(
		uint16_t bytecode_len,
		uint16_t args_stack_length,
		const std::map<SemanticVarPtr, size_t>& base_var_map,
		const std::vector<uint16_t>& arg_stack_offsets
		){

	bytecode = new uint8_t[bytecode_len];
	// cout << "bytecode:" << uint64_t(bytecode) << endl;
	uint8_t* bc_head = bytecode;
	// size_t bytecode_len = 0;
	// size_t stack_offset = args_stack_length;
	auto [_, stack_size] = _write_bytecode(
		this, bytecode, args_stack_length,
		base_var_map, arg_stack_offsets);

	// this->bytecode = bytecode;
	bytecode_end = bytecode + bytecode_len;
	// this->stack_size = stack_size;
}







// void Func::write_bytecode(
// 		size_t args_stack_length,
// 		const std::map<void*, size_t>& base_var_map,
// 		const std::vector<uint16_t>& arg_stack_offsets,
// 		){

// 	uint8_t* bytecode = new uint8_t[bytecode_len];
// 	uint8_t* bc_head = bytecode;
// 	// size_t bytecode_len = 0;
// 	size_t stack_offset = args_stack_length;

// 	using stack_tuple = std::tuple<Func*, int> ;
// 	std::vector<stack_tuple> stack = {};
// 	Func* cf = (Func*) this;
// 	int i = 0;	
// 	bool keep_looping = true;	
// 	while(keep_looping){
// 		if(i < cf->root_arg_infos.size()){
// 			auto& arg_info = cf->root_arg_infos[i];
// 			if(arg_info.kind == ARGINFO_FUNC){
// 				// Push stack
// 				stack.emplace_back(stack_tuple(cf, i+1));
// 				cf = (*cf->get(i)).as_func();
// 				i = 0;
// 			}else{
// 			 switch(arg_info.kind){
// 				case ARGINFO_CONST:
// 					write_load_const(bc_head, i, stack_offset);
// 					bc_head += sizeof_load_const();
// 					stack_offset += arg_info.type->byte_width;
// 					break;
// 				case ARGINFO_VAR:
// 				{
// 					Var* var =  (*cf->get(i)).as_var();
// 					if(var->size() > 0){
// 						write_deref_var(bc_head, var, root_info.offset, stack_offset);
// 						bc_head += sizeof_load_const();
// 						stack_offset += arg_info.type->byte_width;
// 					}
// 					break;
// 				}
// 				default:
// 					throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
// 			}}
// 			++i;
// 		}

// 		while(i >= cf->root_arg_infos.size()){
// 			write_call_func(bc_head, cf, stack_offset, head_stack_offsets);
// 			bc_head += sizeof_call_func(cf);
// 			stack_length += cf->return_type->byte_width;

// 			// Check for end case
// 			if(stack.size() == 0){
// 				keep_looping = false;
// 				break;
// 			}
			
// 			// Pop stack
//             std::tie(cf, i) = stack[stack.size()-1];
//             stack.pop_back();
// 		}
// 	}
// }


// void Func::write_bytecode(){
// 	size_t bytecode_len = 0;
// 	size_t stack_length = 0;

// 	using stack_tuple = std::tuple<Func*, int> ;
// 	std::vector<stack_tuple> stack = {};
// 	Func* cf = (Func*) this;
// 	int i = 0;	
// 	bool keep_looping = true;	
// 	while(keep_looping){
// 		if(i < cf->root_arg_infos.size()){
// 			auto& arg_info = cf->root_arg_infos[i];
// 			if(arg_info.kind == ARGINFO_FUNC){
// 				// Push stack
// 				stack.emplace_back(stack_tuple(cf, i+1));
// 				cf = (*cf->get(i)).as_func();
// 				i = 0;
// 			}else{
// 			 switch(arg_info.kind){
// 				case ARGINFO_CONST:
// 					bytecode_len += sizeof_load_const();
// 					break;
// 				case ARGINFO_VAR:
// 				{
// 					Var* var =  (*cf->get(i)).as_var();
// 					if(var->size() > 0){
// 						bytecode_len += sizeof_deref_var(var);	
// 					}
// 					break;
// 				}
// 				default:
// 					throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
// 			}}
// 			++i;
// 		}

// 		while(i >= cf->root_arg_infos.size()){
// 			bytecode_len += sizeof_call_func(cf);

// 			// Check for end case
// 			if(stack.size() == 0){
// 				keep_looping = false;
// 				break;
// 			}
			
// 			// Pop stack
//             std::tie(cf, i) = stack[stack.size()-1];
//             stack.pop_back();
// 		}
// 	}
// 	// cout << endl;
// 	return {bytecode_len, stack_length};
// }

// size_t Func::write_bytecode(){
// 	size_t bytecode_len = 0
// 	size_t stack_length = 0

// 	using stack_tuple = std::tuple<Func*, int> ;
// 	std::vector<stack_tuple> stack = {};
// 	Func* cf = (Func*) this;
// 	int i = 0;	
// 	bool keep_looping = true;	
// 	while(keep_looping){
// 		if(i < cf->root_arg_infos.size()){
// 			auto& arg_info = cf->root_arg_infos[i];
// 			if(arg_info.kind == ARGINFO_FUNC){
// 				// Push stack
// 				stack.emplace_back(stack_tuple(cf, i+1));
// 				cf = (*cf->get(i)).as_func();
// 				i = 0
// 				break;
// 			}else{
// 			 switch(arg_info.kind){
// 				case ARGINFO_CONST:
// 					bytecode_len += sizeof_load_const();
// 					break;
// 				case ARGINFO_VAR:
// 					Var* var =  (*cf->get(i)).as_var();
// 					if(var->size() > 0){
// 						bytecode_len += sizeof_deref_var(var);	
// 					}
// 					break;
// 				default:
// 					throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind))
// 			}}
// 			++i;
// 		}

// 		while(i >= cf->root_arg_infos.size()){
// 			bytecode_len += sizeof_call_func(cf);

// 			// Check for end case
// 			if(stack.size() == 0){
// 				keep_looping = false;
// 				break;
// 			}
			
// 			// Pop stack
//             std::tie(cf, i) = stack[stack.size()-1];
//             stack.pop_back();
// 		}
// 	}
// 	// cout << endl;
// 	return bytecode_len;
// }


// void 

// void copy_bytecode(
// 	Func* src_func,
// 	uint8_t* dest_bc, 
// 	uint16_t section_offset,
// 	const std::map<void*, size_t>& base_var_map) {

// 	src_base_funcs
// 	uint8_t* bc = src_func->bytecode;
// 	uint8_t* dc = dest_bc;


// 	// cout << "LEN: " << bytecode_end-bc << endl;
// 	while(bc < bytecode_end){


// 		uint16_t* rc_instr = (uint16_t*) bc;
// 		uint16_t* dest_instr = (uint16_t*) dc;


// 		switch(rc_instr[0]){
// 			dest_instr[0] = rc_instr[0]; // Kind
// 			dest_instr[1] = rc_instr[1]; // Instr Size 

// 			case BC_READ_CONST:
// 				// dest_instr[1] = rc_instr[1];
// 				dest_instr[2] = rc_instr[2];
// 				dest_instr[3] = rc_instr[3] + section_offset;
// 				// ss << fmt::format("CONST: [{}]->@{}\n", rc_instr[2], rc_instr[3]);
// 				break;
// 			case BC_DEREF_VAR:

// 				uint deref_size = (rc_instr[1]-8)
// 				 // next offset
// 				dest_instr[2] = ?? // Source Offset
// 				dest_instr[3] = rc_instr[3] + section_offset // Dest Offset


// 				rc_instr[1] = sizeof_deref_var(var);
// 				rc_instr[2] = src_offset;
// 				rc_instr[3] = dest_offset;
// 				DerefInfo* src_df_info = (DerefInfo*) &rc_instr[4];
// 				DerefInfo* dst_df_info = (DerefInfo*) &dest_instr[3];
// 				memcpy(dst_df_info, src_df_info, deref_size);

// 				// ss << fmt::format("DEREF: @{}->@{}\n", rc_instr[2], rc_instr[3]);
// 				break;
// 			case BC_CALL_FUNC:
// 			{
// 				std::vector<std::string> arg_strs = {};
// 				uint16_t n_args = rc_instr[3];
// 				Func* cf = *((Func**) &rc_instr[4]);
// 				// cout << "::" << uint64_t(cf) << endl;
// 				for(int i=0; i < n_args; ++i){ 
// 					rc_instr[8+i]
// 					// arg_strs.push_back(fmt::format("@{}", rc_instr[8+i]));
// 				}
				
// 				// ss << fmt::format("CALL : {}({})->@{}\n", cf->origin_data->name, fmt::join(arg_strs, ", "), rc_instr[2]);
// 				break;
// 			}
// 		}
// 		// cout << rc_instr[0] << " BC: " << uint64_t(bc) << ", " << rc_instr[1] << endl;
// 		if(rc_instr[1] == 0){
// 			break;
// 		}
// 		bc += rc_instr[1];
// 		dc += rc_instr[1];
// 	}

// }




// void Func::fill_byte_code(){

	

// 	// Fill in the bytecode
// 	uint8_t* bytecode = new uint8_t[bytecode_len];
// 	uint8_t* bc_head = bytecode;
// 	for(size_t i=0; i < this->root_arg_infos.size(); ++i){
// 		auto arg_info = this->root_arg_infos[i];
// 		switch(arg_info.kind){
// 			case ARGINFO_CONST:
// 				write_load_const(bc_head, i, ??dest);
// 				bc_head += sizeof_load_const();
// 			case ARGINFO_VAR:
// 				Var* var = this->get(i)->as_var();
// 				if(var.size() > 0){
// 					write_deref_var(bc_head, ??src, ??dest);
// 					bc_head += sizeof_deref_var(var);	
// 				}
// 			case ARGINFO_FUNC:
// 				Func* cf = this->get(i)->as_func();
// 				write_call_func(bc_head, ??src, ??dest);
// 				bc_head += sizeof_call_func(cf);	
// 		}
// 	}
// }





std::ostream& operator<<(std::ostream& out, const StrBlock& sb){
	return out << sb.view;
}

// ---------------------------------------------------------------
// reinitialize()


void Func::reinitialize(){
	// cout << "REINIT FUNC: " << uint64_t(this) << endl;


	using base_heads_pair_t = std::tuple<void*,std::vector<HeadInfo>>;
	if(this->is_initialized) return;

	uint16_t max_arg_depth = 0;


	// Build new base_var_map. This dictates a new signature when there are  
	//  repeated Vars in a Func's expression. For instance, A=Var("A", int),
	//  f = add(A, A), maps two args to one for an effective signature of f(A). 
	std::map<SemanticVarPtr, size_t> base_var_map = {};
	std::vector<base_heads_pair_t> base_vars = {};
	size_t n_vars = 0;


	
	

	// ArgInfo& last_arg = root_arg_infos[root_arg_infos.size()-1];
	// size_t bytecode_len = 0;
	uint16_t args_stack_length = 0; //last_arg.offset + last_arg.type.byte_width;
	std::vector<uint16_t> arg_stack_offsets = {};
	
	auto _try_insert_var = [&](HeadInfo& head_info) {
		Var* var = head_info.var_ptr;
		SemanticVarPtr base_ptr = SemanticVarPtr(var->base);
		auto [it, inserted] = base_var_map.try_emplace(base_ptr, n_vars);
		if(inserted){
			std::vector<HeadInfo> var_head_info = {};
			base_vars.push_back({base_ptr, var_head_info});
			++n_vars;

			arg_stack_offsets.push_back(args_stack_length);
			args_stack_length += head_info.base_type->byte_width;
		}else{
			// If a base var was used with the same alias as a previously used base var,
			//  but it's type is undefined then swap out the base for the well-defined one.
			if(uint64_t(var->base) != uint64_t(it->first.var_ptr)){
				cout << "SWAP BASE: " << var->repr() << " " << it->first.var_ptr->repr() << endl;
				if(var->length > 0){
					var = it->first.var_ptr;
					var = var->_extend_unsafe(var->deref_infos, var->length);
				}else{
					var = it->first.var_ptr;
				}
				head_info.var_ptr = var;
				head_info.cf_ptr->set(head_info.arg_ind, Item(var));
				// cout << "NEW Var: " << var->repr() << endl;
			}
		}
		std::get<1>(base_vars[it->second]).push_back(head_info);
	};

	// Calculate arg_stack_offsets  
	for(auto hrng : head_ranges){
	// for(size_t i=0; i<head_ranges.size(); ++i){
	// 	auto hrng = head_ranges[i];
	// 	auto hrng = [i];

		for(uint16_t j=hrng.start; j < hrng.end; ++j){


			HeadInfo& head_info = head_infos[j];
			// ArgInfo root_info = root_arg_infos[head_info.arg_ind];

			// cout << "KIND:" << uint64_t(head_info.kind) << endl;
			
			switch(head_info.kind){
				// For ARGINFO_VAR kinds insert base_ptr into the base_var_map
				case ARGINFO_CONST: {
					break;
				}

				case ARGINFO_VAR: {				
					_try_insert_var(head_info);
					break;
				}

			// For ARGINFO_FUNC_UNEXPANDED kind insert the base_ptrs of all of the
            //  CRE_Funcs's base vars into base_var_map
				case ARGINFO_FUNC_UNEXPANDED: {

					Func* cf = head_info.cf_ptr;

					for(auto& hrng_k : cf->head_ranges){
						// cout << "HRNG" << endl;
						for(uint16_t n=hrng_k.start; n < hrng_k.end; ++n){
							// HeadInfo& head_info_n = cf->head_infos[n];
							_try_insert_var(cf->head_infos[n]);

						}
						max_arg_depth = std::max(cf->depth, max_arg_depth);
					}
					// bytecode_len += sizeof_call_func(cf);	
				break;
				}
			}
		}
	}
	// bytecode_len += sizeof_call_func(this);	


	


	// Build the Func's bytecode, the instruction sequence
	//  that is called when it is executed
	//size_t bytecode_len = 0//this->calc_byte_code_size();
	// uint8_t* bytecode = new uint8_t[bytecode_len];
	// uint8_t* bc_head = bytecode;

	// std::vector<uint16_t> head_stack_offsets = {};
	// cout << "+++++++++++" << endl;
	// cout << this << endl;
	// for(uint16_t i=0; i < head_ranges.size(); ++i){
	// 	auto hrng = head_ranges[i];
	// // for(auto hrng : head_ranges){

	// 	// cout << "i=" << i << "  stack_offset:" << stack_offset <<  endl;
	// 	for(uint16_t j=hrng.start; j < hrng.end; ++j){
	// 		HeadInfo head_info = head_infos[j];
	// 		ArgInfo root_info = root_arg_infos[head_info.arg_ind];
			
	// 		cout << "i=" << i << " j=" << j << " kind=" << uint32_t(head_info.kind) <<  endl;
	// 		switch(head_info.kind){
	// 			// For ARGINFO_CONST copy a constant into stack
	// 			case  ARGINFO_CONST: {	
	// 			// TODO should attempt any dereferences if necessary
				

	// 			// Write bytecode
	// 			// write_load_const(bc_head, i, root_info.offset);
	// 			write_load_const(bc_head, i, stack_offset);
	// 			bc_head += sizeof_load_const();

	// 			head_stack_offsets.push_back(stack_offset);
	// 			stack_offset += head_info.head_type->byte_width;
	// 			break;

	// 			}
	// 		// For ARGINFO_VAR kinds insert base_ptr into the base_var_map
	// 			case ARGINFO_VAR: {				
	// 			// Write bytecode 
	// 			Var* var = head_info.var_ptr;
	// 			if(var->size() > 0){
	// 				write_deref_var(bc_head, var, root_info.offset, stack_offset);
	// 				bc_head += sizeof_deref_var(var);	

	// 				head_stack_offsets.push_back(stack_offset);
	// 				stack_offset += head_info.head_type->byte_width;
	// 			}else{
	// 				size_t ind = base_var_map[var->base];
	// 				// cout << "arg_stack_offsets[i]" << arg_stack_offsets[ind] << ", " << arg_stack_offsets.size() <<endl;
	// 				head_stack_offsets.push_back(arg_stack_offsets[ind]);
	// 			}
	// 			break;
	// 			}

	// 		// For ARGINFO_FUNC_UNEXPANDED kind insert the base_ptrs of all of the
    //         //  CRE_Funcs's base vars into base_var_map
	// 			case ARGINFO_FUNC_UNEXPANDED: {
	// 			Func* cf = head_info.cf_ptr;
	// 			for(auto& hrng_k : cf->head_ranges){
	// 				for(uint16_t n=hrng.start; n < hrng.end; ++n){
	// 					//
	// 				}
	// 			}
	// 			break;
	// 			}
	// 		}
	// 	}
	// }
	// this->bytecode = bytecode;
	// this->bytecode_end = bytecode+bytecode_len;


	// // Write Bytecode for this Func's call 
	// write_call_func(bc_head, this, stack_offset, head_stack_offsets);
	// stack_offset += return_type->byte_width;
	// // bc_head += sizeof_call_func(cf);
	uint16_t bc_len = this->calc_bytecode_length();
	// args_size = args_stack_length;
	this->write_bytecode(bc_len, args_stack_length, base_var_map, arg_stack_offsets);

	// cout << "--+++++++--" << bc_len <<  endl;
    // Make new head_ranges (spans of HeadInfos for same base)
	//   according to base_var_map. Count total to reserve head_infos.
	size_t n_bases = base_var_map.size();
	std::vector<HeadRange> new_head_ranges;
	new_head_ranges.reserve(n_bases);
	size_t n_heads = 0;
	for(auto [_, base_head_infos] : base_vars){
		// cout << "SIZE:" << base_head_infos.size() << endl;
		new_head_ranges.emplace_back(n_heads, n_heads+base_head_infos.size());
		n_heads += base_head_infos.size();
	}

	// Make new head_infos according to base_var_map
	std::vector<HeadInfo> new_head_infos;
	new_head_infos.reserve(n_heads);

	size_t head_ind=0;
	size_t head_stack_size=0;
	for(auto [base_ptr, base_head_infos] : base_vars){
	// for(size_t i=0; i<base_vars.size(); i++){
		// auto [base_ptr, base_head_infos] = base_vars[i];
		Var* base_var = (Var *) base_ptr;
		for(HeadInfo& base_head_info : base_head_infos){
			HeadInfo hi = base_head_info;
			hi.base_type = base_var->base_type;
			head_stack_size += hi.head_type->byte_width;

			if(hi.head_type->dynamic_dtor != nullptr){
    		   	has_outer_cleanup = true;
    		}

			ArgInfo& root_arg_info = hi.cf_ptr->root_arg_infos[hi.arg_ind];
			root_arg_info.head_ind = head_ind;
			// cout << "hi " << hi.kind << endl;
			new_head_infos.push_back(hi);
			++head_ind;
		}
	}



	this->n_args = n_bases;
	this->head_infos = new_head_infos;
	this->head_ranges = new_head_ranges;
	// this->return_stack_offset = head_stack_size;
	this->outer_stack_size = head_stack_size + return_type->byte_width;
	// cout << "n_bases: " << n_bases << endl;


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
    this->is_initialized = true;

    // cout << "-----" << endl;
}


// void write_args_to_stack(){

// }


void* Func::call_stack(uint8_t* stack){
	uint8_t* bc = bytecode;

	// cout << "bc:" << uint64_t(bc) << endl;

	if(bytecode == nullptr){
		throw std::runtime_error("Called Func with uninitialized bytecode.");
	}

	void* ret_ptr;

	while(bc < bytecode_end){
		uint16_t* rc_instr = (uint16_t*) bc;	
		switch(rc_instr[0]){
			case BC_READ_CONST:
				
				break;
			case BC_DEREF_VAR:
				
				break;
			case BC_CALL_FUNC:
			{
				// std::vector<std::string> arg_strs = {};
				uint16_t ret_offset = rc_instr[2];
				uint16_t n_args = rc_instr[3];
				Func* cf = *((Func**) &rc_instr[4]);


				// cout << "ret_offset:" << uint64_t(ret_offset) << endl;
				// cout << "n_args:" << uint64_t(n_args) << endl;
				// cout << "cf:" << uint64_t(cf) << endl;

				ret_ptr = (void *) (stack + ret_offset);
				void** arg_ptrs = (void**) alloca(sizeof(void*) * n_args);
				for(size_t i=0; i < n_args; ++i){
					arg_ptrs[i] = stack + rc_instr[8+i];
					// cout << "::" << uint64_t(arg_ptrs[i]) << endl;
				}



				// void (*_call_stack)(void**) = (void()(void**)) call_heads_addr;
				// _call_stack(ret_ptr, arg_ptrs);
				stack_call_func(ret_ptr, arg_ptrs);


				// cout << "AFTER: " << uint64_t(ret_ptr) << endl;
				// for(int i=0; i < n_args; ++i){ 
				// 	arg_strs.push_back(fmt::format("@{}", rc_instr[8+i]));
				// }
				
				// ss << fmt::format("CALL : {}({})->@{}\n", cf->origin_data->name, fmt::join(arg_strs, ", "), rc_instr[2]);
				break;
			}
		}
		// End case, null terminated
		if(rc_instr[1] == 0){
			break;
		}

		// Get next bytecode
		bc += rc_instr[1];
	}

	return ret_ptr;
	// return ss.str();
}



void Func::call_recursive2(void* ret_ptr, uint8_t* input_args){
	if(is_composed){
		uint8_t* args_stack = (uint8_t*) alloca(stack_size);
		_call_recursive2(ret_ptr, args_stack, input_args);
	}else{
		stack_call_func2(ret_ptr, input_args);
	}
}

void Func::_call_recursive2(void* ret_ptr,
						   uint8_t* args_stack,
						   uint8_t* input_args){

	for(size_t i=0; i < root_arg_infos.size(); ++i){
		auto& arg_info = root_arg_infos[i];
		switch(arg_info.kind){

		case ARGINFO_FUNC:
		{
			Func* func =  (*this->get(i))._as<Func*>();
			uint8_t* _args_stack = (uint8_t*) alloca(func->stack_size);
			void* _ret_ptr = (void*) &args_stack[arg_info.offset];
			func->_call_recursive2(_ret_ptr, _args_stack, input_args);
			break;
		}
		case ARGINFO_CONST:
		{
			Item* const_val = get(i);
			memcpy(&args_stack[arg_info.offset], &(const_val->val), arg_info.type->byte_width);
			break;
		}
		case ARGINFO_VAR:
		{
			Var* var =  (*get(i))._as<Var*>();
			if(var->size() == 0){
				break;
				// memcpy(&args_stack[arg_info.offset],
				//  	   &input_args[arg_info.base_offset],
				//  	   arg_info.type->byte_width);
			}else{
				break; // Deref chain
			}
			break;
		}
		default:
			throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
		}
	}

	stack_call_func2(ret_ptr, args_stack);
}


void Func::call_recursive(void* ret_ptr, void** head_val_ptrs){
	if(is_composed){
		_call_recursive(ret_ptr, head_val_ptrs);
	}else{
		stack_call_func(ret_ptr, head_val_ptrs);
	}
}

void Func::_call_recursive(void* ret_ptr,
						   void** head_val_ptrs){

	uint8_t* inter_stack = (uint8_t*) alloca(stack_size); // TODO:
	void** arg_ptrs = (void**) alloca(sizeof(void**)*n_root_args);
	// uint8_t* inter_stack = (uint8_t*) alloca(stack_size); // TODO:

	// cout << "SIZEOF OBJ:: " << sizeof(CRE_Obj) << endl;
	// cout << "SIZEOF STR:: " << sizeof(std::string) << endl;
	// cout << "SIZEOF STR_VIEW:: " << sizeof(std::string_view) << endl;
	cout << "R: STACK SIZE: " << stack_size << endl;
	cout << "R: STACK: " << uint64_t(inter_stack) << endl;
	// cout << "THIS: " << uint64_t(this) << endl;
	// cout << "HEAD VAL PTRS 0: " << uint64_t(head_val_ptrs[0]) << endl;

	for(size_t i=0; i < n_root_args; ++i){
		auto& arg_info = root_arg_infos[i];
		switch(arg_info.kind){

		case ARGINFO_FUNC:
		{
			Func* func = this->get(i)->_as<Func*>();
			// cout << "Func: " << uint64_t(func) << endl;
			// uint8_t* _args_stack = (uint8_t*) alloca(func->args_size);
			// void* _ret_ptr = alloca()//(void*) &args_stack[arg_info.offset];
			cout << "OFFSET: " << arg_info.offset << endl;
			void* _ret_ptr = (void*) (inter_stack + arg_info.offset); // TODO +??
			arg_ptrs[i] = _ret_ptr;

			cout << "ADDR B:" << uint64_t(_ret_ptr) << ", " << uint64_t(inter_stack) << endl;
			func->_call_recursive(_ret_ptr, head_val_ptrs);


			cout << "ADDR A:" << uint64_t(_ret_ptr) << endl;
			cout << "VAL:" << ((StrBlock*) _ret_ptr)->view << endl;

			// cout << "Func val: " << int64_t(*((int64_t*)_ret_ptr)) << endl << endl;
			break;
		}
		case ARGINFO_CONST:
		{
			Item* const_val = get(i);
			// cout << "CONST: " <<  const_val->val << endl;
			arg_ptrs[i] = (void*) &(const_val->val);
			// memcpy(&args_stack[arg_info.offset], &(const_val->val), arg_info.type->byte_width);
			break;
		}
		case ARGINFO_VAR:
		{
			// Var* var =  (*get(i)).as_var();
			// cout << "Var i: " << arg_info.var_ind << " Head i: " <<  arg_info.head_ind << endl;
			// cout << "Var val addr: " <<  uint64_t(head_val_ptrs[arg_info.head_ind]) << endl;
			// cout << "Var val: " <<  *((uint64_t*) head_val_ptrs[arg_info.head_ind]) << endl;
			// NOTE: has_deref doesn't matter because input_arg_ptrs
			//  should have already resolved dereferences.
			arg_ptrs[i] = head_val_ptrs[arg_info.head_ind];


			// if(!arg_info.has_deref){
			// 	// arg_ptrs[i] = input_arg_ptrs[arg_info.var_ind];
			// 	break;
			// 	// memcpy(&args_stack[arg_info.offset],
			// 	//  	   &input_args[arg_info.base_offset],
			// 	//  	   arg_info.type->byte_width);
			// }else{

			// 	break; // Deref chain
			// }
			break;
		}
		default:
			throw std::runtime_error("Unrecognized arg_info.kind="+std::to_string(arg_info.kind));
		}
	}

	cout << "INTO CALL FUNC: " << uint64_t(ret_ptr) << endl;
	for(size_t i=0; i < n_root_args; ++i){
		cout << "ARG:" << ((StrBlock*) arg_ptrs[i])->view << endl;
	}
	stack_call_func(ret_ptr, arg_ptrs);
	cout << "RET VAL:" << ((StrBlock*) ret_ptr)->view << endl;
	// cout << "INTO CALL FUNC: " << uint64_t(ret_ptr) << endl;

	// Cleanup
	for(size_t i=0; i < n_root_args; ++i){
		//
	}
}

bool funcs_equal(const Func* func1, const Func* func2, bool semantic, bool castable){
	if(func1->n_args != func2->n_args) return false;
	if(func1->origin_data != func2->origin_data) return false;
	if(func1->n_root_args != func2->n_root_args) return false;
	
	for(size_t i=0; i < func1->n_root_args; ++i){
		const ArgInfo& arg_info1 = func1->root_arg_infos[i];
		const ArgInfo& arg_info2 = func2->root_arg_infos[i];
		if(arg_info1.kind != arg_info2.kind) return false;
		switch(arg_info1.kind){
		case ARGINFO_FUNC:
		{
			Func* func_arg1 = (*func1->get(i))._as<Func*>();
			Func* func_arg2 = (*func2->get(i))._as<Func*>();
			if(!funcs_equal(func_arg1, func_arg2, semantic, castable)) return false;
			break;
		}
		case ARGINFO_VAR:
		{
			Var* var_arg1 = (*func1->get(i))._as<Var*>();
			Var* var_arg2 = (*func2->get(i))._as<Var*>();
			if(arg_info1.var_ind != arg_info2.var_ind) return false;
			if(!vars_equal(var_arg1, var_arg2, semantic, semantic, castable)) return false;
			break;
		}
		case ARGINFO_CONST:
		{
			Item* const_arg1 = func1->get(i);
			Item* const_arg2 = func2->get(i);
			if(!items_equal(*const_arg1, *const_arg2)) return false;
			break;
		}
		}
	}
	return true;

}


bool Func::operator==(const Func& other) const {
	return funcs_equal((Func*) this, (Func*) &other, false);
}


FuncRef Func::compose_args(std::vector<Item>& args, AllocBuffer* alloc_buffer){
    return compose_args(args.data(), args.size(), alloc_buffer);
}

FuncRef Func::compose_args(Item* args, size_t _n_args, AllocBuffer* alloc_buffer){
    FuncRef cf = this->copy_deep(alloc_buffer);
    for(size_t i=0; i < _n_args; ++i){
        cf->set_arg(i, args[i]);
    }
    cf->reinitialize();
    return cf;
}

Item Func::call_heads(void** head_val_ptrs){
	uint8_t* ret_ptr = (uint8_t*) alloca(return_type->byte_width);
	call_recursive_fc(this, ret_ptr, head_val_ptrs);
	return ptr_to_item_func(ret_ptr);
}


Item Func::call_args(Item* args, size_t _n_args){
	void** head_val_ptrs = (void**) alloca(sizeof(void**)*head_infos.size());
	uint8_t* stack = (uint8_t*) alloca(outer_stack_size);
	uint8_t* arg_head = stack;

	int64_t ret = 0;
	void* ret_ptr = (void*) (stack+outer_stack_size-return_type->byte_width);

	if(_n_args != n_args){
	  	throw_bad_n_args(_n_args);
	}

	for(size_t i=0; i < _n_args; ++i){
		auto h_start = head_ranges[i].start;
		auto h_end = head_ranges[i].end;

    	for(size_t head_ind=h_start; head_ind<h_end; ++head_ind){

    		const HeadInfo& hi = head_infos[head_ind];
    		void* head_val_ptr = resolve_heads(arg_head, args, hi);

	    	if(head_val_ptr == nullptr){ [[unlikely]]
	    		throw_bad_arg(i, args, hi.base_type);	
	    	}
			
			head_val_ptrs[head_ind] = head_val_ptr;
			arg_head += hi.base_type->byte_width;	    	
		}
    }

  	call_recursive_fc(this, ret_ptr, head_val_ptrs);

    // Cleanup any head values
	if(has_outer_cleanup){
		for(size_t head_ind=0; head_ind < head_infos.size(); ++head_ind){
			const HeadInfo& hi = head_infos[head_ind];
			if(hi.head_type->dynamic_dtor != nullptr){
				// cout << ":::" << uint64_t(hi.head_type->dynamic_dtor) << endl;
				hi.head_type->dynamic_dtor(head_val_ptrs[head_ind]);
			}
		}	
	}
  
  return ptr_to_item_func(ret_ptr);
}

Item Func::call_args(std::vector<Item>& args){
    return call_args(args.data(), args.size());
}
// Example: 

// a + (d + c + 1) + c 
// sig: f(a,d,c,e)
// bc : [a][d][c][1] [add3(@1, @2, @3)] [add3(@0, @4, @2)]

} // NAMESPACE_END(cre)




