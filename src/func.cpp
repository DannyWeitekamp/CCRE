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


namespace cre {

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
	uint16_t offset = 0;
	for(uint32_t i =0; i< arg_types.size(); ++i){
		CRE_Type* arg_type = arg_types[i];

		auto alias = intern(fmt::format("x{}", i));
		ref<Var> var = new_var(alias, arg_type);

		func->set(i, var.get());

		// ArgInfo
		// uint16_t t_id = arg_type->t_id;
		root_arg_infos.emplace_back(
			arg_type, func->get(i), ARGINFO_VAR, offset, false);
		offset += arg_type->byte_width;

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
		// hi.byte_width = 0;
		hi.ref_kind = 0; // Necessary ? 

		head_infos.push_back(hi);

		// Head Range
		head_ranges.emplace_back(i, i+1);
	}
 }

FuncRef define_func(
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
	func->return_type = ret_type;

	return (FuncRef) func;
}

// -------------------------------------------------------------
// copy()
FuncRef Func::copy_shallow(){
	ref<Func> nf = new_func(call_heads_addr, n_root_args, origin_data);
	nf->dtor = &Func_dtor;

	nf->return_type = return_type;
	nf->n_args = n_args;
	nf->n_root_args = n_root_args;

	nf->root_arg_infos = root_arg_infos;
	nf->head_ranges = head_ranges;
	nf->head_infos = head_infos;

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
   

	// cout << ":" << uint64_t(root_arg_infos.data()) << endl;
	// cout << ":" << uint64_t(nf->root_arg_infos.data()) << endl;

	nf->is_initialized = false;	
	nf->is_ptr_func = is_ptr_func;	
	nf->depth = depth;	
	nf->has_any_derefs = has_any_derefs;	
	nf->is_composed = is_composed;	
	return (FuncRef) nf;
}

FuncRef Func::copy_deep(){


	ref<Func> cpy;
	if(depth <= 1){
		cpy = copy_shallow();
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
			cf = (Func*) arg_info.ptr->as_func();
			func_copies = new std::vector<ref<Func>>;
			i =0;
		}else{
			i += 1;
		}
	

		// When past end arg of deepest func, copy it. Then pop prev frame from stack.
		
		while(i >= cf->root_arg_infos.size()){
	        cpy = cf->copy_shallow();
	    	size_t j = 0;
	    	for(size_t k=0; k < cpy->root_arg_infos.size(); ++k){
	    		ArgInfo& inf = cpy->root_arg_infos[k];
	    		if(inf.kind == ARGINFO_FUNC){
	    			Func* sub_func = (*func_copies)[j];
	    			cpy->set(k, sub_func);
	    			inf.ptr = cpy->get(k); 
	    			j += 1;
	    		}else{
	    			cpy->set(k, *cf->get(k));
	    		}
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

	// cout << "TO STRING: " << uint64_t(this) << endl;

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
		// cout << "S LOOP: " << stack.size() << ", " << i << endl;

		if(i < cf->root_arg_infos.size()){
			auto& arg_info = cf->root_arg_infos[i];

			// cout << ">> i=" << i << ", kind=" << arg_info.kind << endl;

			if(arg_info.kind == ARGINFO_FUNC){
				// cout << "FUNC0 :" << uint64_t(cf) << endl;
				
				// Recurse Case: Func

				stack.emplace_back(stack_tuple(cf, i+1, arg_strs));

				// cf = cf->get(i).as_func(); 
				cf = (*arg_info.ptr).as_func();
				// cout << "FUNC  :" << uint64_t(cf) << endl;
				arg_strs = new fmt_args_t();
				i =0;
			}else{
				// Terminal Case: Var/Const
				if(arg_info.kind == ARGINFO_VAR){
					// cout << "VAR:" << uint64_t(arg_info.ptr) << endl;
					Var* var = (*arg_info.ptr).as_var();
					
					if(use_derefs){
						arg_strs->push_back(var->to_string());
					}else{
						arg_strs->push_back(var->alias.to_string());
					}
				}else if(arg_info.kind == ARGINFO_CONST){
					// cout << "CONST:" << uint64_t(arg_info.ptr) << endl;
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
	cout << "SIZE" << this->head_ranges.size() << endl;

	if(i >= this->head_ranges.size()){
		throw std::invalid_argument("Setting Func arg out of range.");
	}


	this->is_initialized = false;
	// auto& head_infos = this->head_infos;
	size_t start = this->head_ranges[i].start;
	size_t end = this->head_ranges[i].end;

	uint8_t kind = val.t_id == T_ID_VAR ? ARGINFO_VAR :
				   val.t_id == T_ID_FUNC ? ARGINFO_FUNC :
				   ARGINFO_CONST;

	cout << uint64_t(this) << "  set i=" << i << ", val=" << val << ", t_id=" << uint64_t(val.t_id) << ", kind=" << uint64_t(kind) << endl;

	// if(!kind){
	// 	throw std::invalid_argument(
	// 	"Invalid value provided for Func::set_arg(), but got " + val.to_string() + 
	// 	" with t_id=" + std::to_string(val.t_id) + ". Expected int, float, string, Var or Func.");	
	// }


	
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
        		Var* var = val.as_var();
        		// TODO: Check output Type
        		// if(head_info.base_t_id != var->head_t_id && !this->is_ptr_func){
            	// 	throw std::invalid_argument("Var's head_type doesn't match composing CREFunc's argument type.");
        		// }

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
		            cf->set(arg_ind, val);
		        }

		        head_info.has_deref = has_deref;
		        head_info.var_ptr = head_var.get();
		    }
        		break;
        	case ARGINFO_FUNC:
        		// TODO check func return type
        		head_info.kind = ARGINFO_FUNC_UNEXPANDED;
        		head_info.cf_ptr = val.as_func();
        		cf->set(arg_ind, val);
        		break;
        	default:
        		cf->set(arg_ind, val);
        		// Do nothing
       	}
		head_info.has_deref = has_deref;

		

		// cout << "set j=" << j << ", arg_ind=" << arg_ind << endl;

		cf->root_arg_infos[arg_ind].kind = kind;
		cf->root_arg_infos[arg_ind].has_deref = has_deref;
		cf->root_arg_infos[arg_ind].ptr = cf->get(arg_ind);
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

void write_call_func(uint8_t* bytecode, Func* cf, uint16_t ret_offset, std::vector<uint16_t>& head_offsets){
	uint16_t* rc_instr = (uint16_t*) bytecode;
	rc_instr[0] = BC_CALL_FUNC;
	rc_instr[1] = sizeof_call_func(cf);
	rc_instr[2] = ret_offset;
	rc_instr[3] = head_offsets.size();
	// cout << "<::" << uint64_t(cf) << endl;
	Func** func_ptr = (Func**) &rc_instr[4];
	*func_ptr = cf;
	// cout << "<::" << uint64_t(*func_ptr) << endl;

	uint16_t* self_arg_offsets = (uint16_t*) &rc_instr[8];
	for(uint16_t i=0; i < head_offsets.size(); i++){
		// TODO
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


size_t Func::calc_byte_code_size(){
	// First pass to find the size of the bytecode
	size_t byte_code_len = 0;
	for(size_t i=0; i < this->root_arg_infos.size(); ++i){
		auto arg_info = this->root_arg_infos[i];
		switch(arg_info.kind){
			case ARGINFO_CONST:
			{
				byte_code_len += sizeof_load_const();
				break;
			}
			case ARGINFO_VAR:
			{
				Var* var = this->get(i)->as_var();
				cout << "#" << uint64_t(var) << endl;
				if(var->size() > 0){
					byte_code_len += sizeof_deref_var(var);	
				}
				break;
			}
			case ARGINFO_FUNC_UNEXPANDED:
			{
				Func* cf = this->get(i)->as_func();
				byte_code_len += sizeof_call_func(cf);	
				break;
			}
		}
	}
	byte_code_len += sizeof_call_func(this);	
	return byte_code_len;
}

std::string Func::bytecode_to_string() {
	uint8_t* bc = bytecode;
	std::stringstream ss;
		
	// cout << "LEN: " << bytecode_end-bc << endl;
	while(bc < bytecode_end){
		uint16_t* rc_instr = (uint16_t*) bc;	
		switch(rc_instr[0]){
			case BC_READ_CONST:
				ss << fmt::format("CONST: [{}]->@{})\n", rc_instr[2], rc_instr[3]);
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

// void Func::fill_byte_code(){

	

// 	// Fill in the bytecode
// 	uint8_t* bytecode = new uint8_t[byte_code_len];
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


// ---------------------------------------------------------------
// reinitialize()

void Func::reinitialize(){

	using base_heads_pair_t = std::tuple<void*,std::vector<HeadInfo>>;
	if(this->is_initialized) return;

	uint16_t max_arg_depth = 0;


	// Build new base_var_map. This dictates a new signature when there are  
	//  repeated Vars in a Func's expression. For instance, A=Var("A", int),
	//  f = add(A, A), maps two args to one for an effective signature of f(A). 
	std::map<void*, size_t> base_var_map = {};
	std::vector<base_heads_pair_t> base_vars = {};
	size_t n_vars = 0;

	

	// ArgInfo& last_arg = root_arg_infos[root_arg_infos.size()-1];
	size_t byte_code_len = 0;
	uint16_t stack_offset = 0; //last_arg.offset + last_arg.type.byte_width;
	std::vector<uint16_t> arg_stack_offsets = {};
	

	for(auto hrng : head_ranges){
		for(uint16_t j=hrng.start; j < hrng.end; ++j){

			HeadInfo head_info = head_infos[j];
			ArgInfo root_info = root_arg_infos[head_info.arg_ind];

			cout << "KIND:" << uint64_t(head_info.kind) << endl;
			
			switch(head_info.kind){
				// For ARGINFO_VAR kinds insert base_ptr into the base_var_map
				case ARGINFO_CONST: {
				// arg_stack_offsets.push_back(stack_offset);
				// stack_offset += head_info.base_type->byte_width;
					byte_code_len += sizeof_load_const();

					break;
				}

				case ARGINFO_VAR: {				
					
					Var* var = head_info.var_ptr;
					void* base_ptr = (void*) var->base;

					cout << "VP:" << uint64_t(head_info.var_ptr) << endl;

					auto [it, inserted] = base_var_map.try_emplace(
						base_ptr, n_vars);
					if(inserted){
						std::vector<HeadInfo> var_head_info = {};
						base_vars.push_back({base_ptr, var_head_info});
						++n_vars;

						// cout << "PB stack_offset:" << stack_offset <<  endl;
						arg_stack_offsets.push_back(stack_offset);
						stack_offset += head_info.base_type->byte_width;
					}
					if(var->size() > 0){
						byte_code_len += sizeof_deref_var(var);	
					}
					std::get<1>(base_vars[it->second]).push_back(head_info);
					break;
				}

			// For ARGINFO_FUNC_UNEXPANDED kind insert the base_ptrs of all of the
            //  CRE_Funcs's base vars into base_var_map
				case ARGINFO_FUNC_UNEXPANDED: {

					Func* cf = head_info.cf_ptr;

					for(auto& hrng_k : cf->head_ranges){
						// cout << "HRNG" << endl;
						for(uint16_t n=hrng_k.start; n < hrng_k.end; ++n){
							HeadInfo& head_info_n = cf->head_infos[n];

							Var* var = head_info_n.var_ptr;
							// cout << var << endl;
							// cout << "n=" << n << " UVP:" << uint64_t(head_info_n.var_ptr) << endl;
							void* base_ptr = (void*) var->base;

							// std::vector<HeadInfo> var_head_info = {};
							auto [it, inserted] = base_var_map.try_emplace(
								base_ptr, n_vars);
							if(inserted){
								std::vector<HeadInfo> var_head_info = {};
								base_vars.push_back({base_ptr, var_head_info});
								++n_vars;

								arg_stack_offsets.push_back(stack_offset);
								stack_offset += head_info_n.base_type->byte_width;
							}
							std::get<1>(base_vars[it->second]).push_back(head_info_n);							
						}
						max_arg_depth = std::max(cf->depth, max_arg_depth);
					}
					byte_code_len += sizeof_call_func(cf);	
				break;
				}
			}
		}
	}
	byte_code_len += sizeof_call_func(this);	


	


	// Build the Func's bytecode, the instruction sequence
	//  that is called when it is executed
	//size_t byte_code_len = 0//this->calc_byte_code_size();
	uint8_t* bytecode = new uint8_t[byte_code_len];
	uint8_t* bc_head = bytecode;

	std::vector<uint16_t> head_stack_offsets = {};

	for(uint16_t i=0; i < head_ranges.size(); ++i){
		auto hrng = head_ranges[i];
	// for(auto hrng : head_ranges){

		// cout << "stack_offset:" << stack_offset <<  endl;
		for(uint16_t j=hrng.start; j < hrng.end; ++j){
			HeadInfo head_info = head_infos[j];
			ArgInfo root_info = root_arg_infos[head_info.arg_ind];
			
			switch(head_info.kind){
				// For ARGINFO_CONST copy a constant into stack
				case  ARGINFO_CONST: {	
				// TODO should attempt any dereferences if necessary
				

				// Write bytecode
				// write_load_const(bc_head, i, root_info.offset);
				write_load_const(bc_head, i, stack_offset);
				bc_head += sizeof_load_const();

				head_stack_offsets.push_back(stack_offset);
				stack_offset += head_info.head_type->byte_width;
				break;

				}
			// For ARGINFO_VAR kinds insert base_ptr into the base_var_map
				case ARGINFO_VAR: {				
				// Write bytecode 
				Var* var = head_info.var_ptr;
				if(var->size() > 0){
					write_deref_var(bc_head, var, root_info.offset, stack_offset);
					bc_head += sizeof_deref_var(var);	

					head_stack_offsets.push_back(stack_offset);
					stack_offset += head_info.head_type->byte_width;
				}else{
					size_t ind = base_var_map[var->base];
					// cout << "arg_stack_offsets[i]" << arg_stack_offsets[ind] << ", " << arg_stack_offsets.size() <<endl;
					head_stack_offsets.push_back(arg_stack_offsets[ind]);
				}
				break;
				}

			// For ARGINFO_FUNC_UNEXPANDED kind insert the base_ptrs of all of the
            //  CRE_Funcs's base vars into base_var_map
				case ARGINFO_FUNC_UNEXPANDED: {
				Func* cf = head_info.cf_ptr;
				for(auto& hrng_k : cf->head_ranges){
					for(uint16_t n=hrng.start; n < hrng.end; ++n){
						//
					}
				}
				break;
				}
			}
		}
	}
	this->bytecode = bytecode;
	this->bytecode_end = bytecode+byte_code_len;


	// Write Bytecode for this Func's call 
	write_call_func(bc_head, this, stack_offset, head_stack_offsets);
	stack_offset += return_type->byte_width;
	// bc_head += sizeof_call_func(cf);


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
	for(auto [base_ptr, base_head_infos] : base_vars){
		Var* base_var = (Var *) base_ptr;
		for(HeadInfo& base_head_info : base_head_infos){
			HeadInfo hi = base_head_info;
			hi.base_type = base_var->base_type;

			new_head_infos.push_back(std::move(hi));
		}
	}



	this->n_args = n_bases;
	this->head_infos = new_head_infos;
	this->head_ranges = new_head_ranges;
	cout << "n_bases: " << n_bases << endl;


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

    cout << "-----" << endl;
}

// Example: 
// a + (d + c + 1) + c 
// sig: f(a,d,c,e)
// bc : [a][d][c][1] [add3(@1, @2, @3)] [add3(@0, @4, @2)]

} // NAMESPACE_END(cre)




