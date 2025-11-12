#include "../include/var.h"
#include <alloca.h>             // for alloca
#include <fmt/format.h>         // for format
#include <fmt/ranges.h>         // for join, join_view
#include <string.h>             // for size_t, memcpy
#include <iostream>             // for basic_ostream, operator<<, basic_ios
#include <new>                  // for operator new
#include <sstream>              // for basic_stringstream
#include <stdexcept>            // for runtime_error, invalid_argument
#include "../include/fact.h"    // for Fact
#include "../include/member.h"  // for Member
#include "../include/ref.h"     // for ref
#include "cre_obj.h"            // for CRE_Obj (ptr only), alloc_cre_obj
#include "hash.h"               // for CREHash
#include "types.h"              // for FactType, CRE_Type
namespace cre { struct AllocBuffer; }


namespace cre {
// extern "C" 

void Var_dtor(const CRE_Obj* x){
	Var* var = (Var*) x;

	if(var->base != nullptr && var->base != var){
		var->base->dec_ref();
	}

	CRE_Obj_dtor(x);
}


Var::Var(const Item& _alias,
 		CRE_Type* _type,
 		uint8_t _kind,
 		DerefInfo* _deref_infos,
 		size_t _length) : 

		// CRE_Obj(&Var_dtor),
		base(nullptr), 
		base_type(_type), head_type(_type),
		alias(_alias),
		kind(_kind),
		deref_infos(nullptr),
		length(_length),
		hash(0){


	// if(_type == nullptr){
	// 	throw std::invalid_argument("Cannot initialize Var from NULL type.");
	// }
	// this->init_control_block(&Var_dtor, T_ID);

	deref_infos = ((DerefInfo*) (( (char*) this) + sizeof(Var)) );

	// cout << "deref_infos_offset=" << uint64_t(deref_infos) - uint64_t(this) << endl;

	if(_deref_infos != nullptr && _length > 0){
		head_type = _deref_infos[_length-1].deref_type;
		memcpy(deref_infos, _deref_infos, _length*sizeof(DerefInfo));
	}


	if(_alias.get_t_id() != T_ID_STR &&
	   _alias.get_t_id() != T_ID_INT &&
	   _alias.get_t_id() != T_ID_UNDEF){
		std::stringstream ss;
		ss << "Var alias must be string or integer. Got: " << _alias << ".";
		throw std::invalid_argument(ss.str());
	}
	base = this;
	
	// cout << "Var_ctor:" << _alias << endl;
}

// Var::Var(const std::string_view& _alias,
// 		CRE_Type* _type,
// 		DerefInfo* _deref_infos,
// 		size_t _length) : 

// 		Var(Item(intern(_alias)), _type, _deref_infos, _length) {
// }

// void _init_var(Var* var,
// 			CRE_Type* _type,
//  			std::string_view _alias,
//  			DerefInfo* _deref_infos,
//  			size_t _length,
//  			size_t copy_len=-1){
// 	var->alias = intern(_alias);
// 	var->deref_infos = ((DerefInfo*) ((char*) var) + sizeof(Var));
// 	var->length = _length;
// 	var->base_type = _type;

// 	copy_len = (copy_len == -1) ? _length : copy_len;
// 	// cout << "INIT: " << uint64_t(copy_len) << ", " <<  uint64_t(_length) << endl; 
// 	if(_length == 0 || copy_len < _length){
// 		var->head_type = _type;
// 	}else{
// 		var->head_type = _deref_infos[copy_len-1].deref_type;
// 		memcpy(var->deref_infos, _deref_infos, copy_len*sizeof(DerefInfo));
// 	}
// 	// cout << "BASE: " << uint64_t(var->base_type) << endl; 	
// 	// cout << "HEAD: " << uint64_t(var->head_type) << endl; 	
// }

ref<Var> new_var(
			const Item& alias,
			CRE_Type* type,
			uint8_t kind,
 			DerefInfo* deref_infos,
 			size_t length,
 			AllocBuffer* buffer){
	

	auto [var_addr, did_malloc] =  alloc_cre_obj(SIZEOF_VAR(length), &Var_dtor, T_ID_VAR, buffer);

	// bool did_malloc = true;
	// Var* var;
	// if(alloc_buffer != nullptr){
	// 	var = (Var*) alloc_buffer->alloc_bytes(SIZEOF_VAR(_length), did_malloc);	
	// }else{
	// 	var = (Var*) malloc(SIZEOF_VAR(_length)); 
	// }
    
    Var* var = new (var_addr) Var(alias, type, kind, deref_infos, length);

    // if(!did_malloc){
    // 	var->control_block->alloc_buffer = alloc_buffer;
    // 	alloc_buffer->inc_ref();
    // }
    
    var->hash = CREHash{}(var);
	// Allocate a new var 
	
	// _init_var(var, _type, _alias, _deref_infos, _length);
	// var->dtor = Var_dtor;
	return var;
}



ref<Var> Not(const Item& alias, CRE_Type* type, DerefInfo* deref_infos, size_t length, AllocBuffer* buffer){
	return new_var(alias, type, VAR_KIND_NOT, deref_infos, length, buffer);
}
ref<Var> Exists(const Item& alias, CRE_Type* type, DerefInfo* deref_infos, size_t length, AllocBuffer* buffer){
	return new_var(alias, type, VAR_KIND_EXIST, deref_infos, length, buffer);
}
ref<Var> Bound(const Item& alias, CRE_Type* type, DerefInfo* deref_infos, size_t length, AllocBuffer* buffer){
	return new_var(alias, type, VAR_KIND_BOUND, deref_infos, length, buffer);
}
ref<Var> Opt(const Item& alias, CRE_Type* type, DerefInfo* deref_infos, size_t length, AllocBuffer* buffer){
	return new_var(alias, type, VAR_KIND_OPTIONAL, deref_infos, length, buffer);
}

// ref<Var> new_var(
// 			const std::string_view& _alias,
// 			CRE_Type* _type,
//  			DerefInfo* _deref_infos,
//  			size_t _length,
//  			AllocBuffer* alloc_buffer){

// 	return new_var(Item(intern(_alias)), _type, _deref_infos, _length, alloc_buffer);
// }



// Var* _alloc_extension(Var* var, size_t extend_by){
// 	Var* new_var = (Var*) malloc(sizeof(Var) + (var->length + extend_by)*sizeof(DerefInfo)); 
// 	memcpy(new_var, var, sizeof(Var) + var->length*sizeof(DerefInfo));
// 	return new_var;
// }

// extern "C" Var* Var_extend_attr(Var* var, std::string_view attr){

	
// }

ref<Var> Var::_extend_unsafe(DerefInfo* derefs, size_t n_derefs, AllocBuffer* alloc_buffer){
	size_t new_len = length+n_derefs;
	auto [var_addr, did_malloc] = alloc_cre_obj(SIZEOF_VAR(new_len), &Var_dtor, T_ID_VAR, alloc_buffer);
	
	ref<Var> nv = new (var_addr) Var(alias, base_type, kind, nullptr, new_len);

	memcpy(nv->deref_infos, deref_infos, length*sizeof(DerefInfo));

	FactType* hf_type = (FactType*) head_type;

	for(size_t i=0; i < n_derefs; ++i){
		// DerefInfo* __restrict new_deref_inf = &nv->deref_infos[length+i];
		nv->deref_infos[length+i] = derefs[i];

	}

	nv->base = this->base; this->base->inc_ref();
	nv->head_type = derefs[n_derefs-1].deref_type;//deref_type;
	nv->length = new_len;
	nv->hash = CREHash{}(nv);
	// _init_var(var, _type, _alias, _deref_infos, _length+1);
	return nv;
}

ref<Var> Var::extend_attr(const std::string_view& attr, AllocBuffer* alloc_buffer){
	// cout << "EXTEND: " << endl;

	if(head_type == nullptr){
		throw std::runtime_error(
			"Cannot extend an untyped Var."
		);
	}

	if(head_type->builtin){
		throw std::runtime_error(
			"Attempting to extend Var " + to_string() + \
			 " with builtin head type " + head_type->name + ".");
	}

	FactType* hf_type = (FactType*) head_type;
	int mbr_ind = hf_type->get_attr_index(attr);

	DerefInfo* deref = (DerefInfo*) alloca(sizeof(DerefInfo));
	deref->deref_type = hf_type->get_item_type(mbr_ind);
	deref->mbr_ind = mbr_ind;
	deref->deref_kind = DEREF_KIND_ATTR;

	// derefs[0] = DEREF_KIND_ATTR;
	return _extend_unsafe(deref, 1, alloc_buffer);
}

ref<Var> Var::extend_item(int16_t mbr_ind, AllocBuffer* alloc_buffer){
	FactType* hf_type = (FactType*) head_type;
	DerefInfo* deref = (DerefInfo*) alloca(sizeof(DerefInfo));
	deref->deref_type = hf_type->get_item_type(mbr_ind);
	deref->mbr_ind = mbr_ind;
	deref->deref_kind = DEREF_KIND_ITEM;
	return _extend_unsafe(deref, 1, alloc_buffer);	
}

void Var::swap_base(Var* new_base){
	new_base->inc_ref();
	if(this->base != nullptr){
		this->base->dec_ref();
	}
	
	base = new_base;
	alias = new_base->alias;
	base_type = new_base->base_type;
	kind = new_base->kind;
	bound_obj = new_base->bound_obj;

	hash = CREHash{}(this);
}

std::string Var::get_alias_str(){
	Item& _alias = length > 0 ? base->alias : alias;
	
	if(_alias.get_t_id() == T_ID_STR){
		return _alias.as<std::string>();
	}else if(_alias.get_t_id() == T_ID_INT){
		return fmt::format("F{}", _alias.as<int64_t>());	
	}else if(_alias.get_t_id() == T_ID_UNDEF){
		if(base_type != cre_undef){
			return fmt::format("Var({})", base_type->to_string());
		}else{
			return "Var()";
		}
	}else{
		throw std::runtime_error("Var has unknown alias type t_id=" + std::to_string(alias.get_t_id()));
	}
	return "";
}

std::string Var::get_deref_str() {
	std::vector<std::string> deref_strs = {};
	deref_strs.reserve(length);

	// std::string out = std::string(var.alias);
	CRE_Type* type = base_type;
	for(int i=0; i < length; i++){
		FactType* fact_type = (FactType*) type;
		DerefInfo inf = deref_infos[i];
		int mbr_ind = int(inf.mbr_ind);
		
		if(inf.deref_kind == DEREF_KIND_ATTR){
			// cout << "mbr_ind=" << int(mbr_ind) <<
			// 	"attr=" << fact_type->get_item_attr(mbr_ind) <<
			//  	endl;
			deref_strs.emplace_back(
				fmt::format(".{}", fact_type->get_item_attr(mbr_ind))
			);
		}else if(inf.deref_kind == DEREF_KIND_ITEM){
			deref_strs.emplace_back(
				fmt::format("[{}]", mbr_ind)
			);
		}
		type = fact_type->get_item_type(mbr_ind);	
	}
	return fmt::format("{}", fmt::join(deref_strs, ""));
}

std::string Var::to_string() {
	return fmt::format("{}{}", get_alias_str(), get_deref_str());		
	
}

std::string Var::get_prefix_str() {
	return VAR_PREFIXES[kind];
}

std::string Var::repr(bool use_alias) {
	std::stringstream ss;
	std::vector<std::string> inner_parts = {};
	if(base_type != cre_undef){
		inner_parts.push_back(base_type->to_string());
	}
	if(use_alias && alias.get_t_id() != T_ID_UNDEF){
		inner_parts.push_back(fmt::format("'{}'", get_alias_str()));
	}
	return fmt::format("{}({}){}", VAR_PREFIXES[kind], fmt::join(inner_parts, ", "), get_deref_str());
}

std::ostream& operator<<(std::ostream& out, Var* var){
	return out << var->to_string();
}

std::ostream& operator<<(std::ostream& out, ref<Var> var){
	return out << var.get()->to_string();
}

Member* deref_once(CRE_Obj* obj, const DerefInfo& inf){
	switch(inf.deref_kind){

	case DEREF_KIND_ATTR:
		{
		Fact* fact = (Fact*) obj;
		return fact->get_ptr(inf.mbr_ind);
		}
	case DEREF_KIND_ITEM:
		std::cerr << "Unimplemented... " << std::endl;
		break;
	default:
		// Handle unknown dereference kind
		std::cerr << "Unknown dereference kind: " << inf.deref_kind << std::endl;
		break;
	}
	return nullptr;
}

Member* deref_multiple(CRE_Obj* obj, DerefInfo* deref_infos, size_t length){
	// cout << "Length: " << length << endl;
	// cout << "OBJ PTR: " << uint64_t(obj) << endl;
	// cout << "OBJ 0 " << ((Fact*) obj)->get("id") << endl;
	// cout << "REF COUNT: " << obj->get_refcount() << endl;
	if(obj == nullptr) return nullptr;

	Member* mbr_ptr = deref_once(obj, deref_infos[0]);
	for(int i=1; i < length; i++){
		obj = mbr_ptr->get_ptr();
		
		if(obj == nullptr) return nullptr;
		// cout << "OBJ PTR: " << uint64_t(obj) << endl;
		// cout << "REF COUNT: " << obj->get_refcount() << endl;
		// cout << "OBJ " << i << " " << ((Fact*) obj)->get("id") << endl;
		mbr_ptr = deref_once(obj, deref_infos[i]);
		
	}
	return mbr_ptr;
}


Item* Var::apply_deref(CRE_Obj* obj){
	return deref_multiple(obj, this->deref_infos, this->length);
}

bool vars_same_type_kind(Var* var1, Var* var2){
	if(var1->kind == var2->kind){
		return (var1->base_type == var2->base_type ||
		        var1->head_type->get_t_id() == T_ID_UNDEF ||
		        var2->head_type->get_t_id() == T_ID_UNDEF) &&
		       (var1->head_type == var2->head_type ||
		        var1->head_type->get_t_id() == T_ID_UNDEF ||
		        var2->head_type->get_t_id() == T_ID_UNDEF);
	}
	return false;
}

bool vars_semantically_equal(Var* var1, Var* var2){
	// cout << "ALIAS:" << var1->alias.val << " " << var2->alias.val << endl;
	if(uint64_t(var1) == uint64_t(var2)) return true;
	
	// Don't use full item equality because aliases of vars are always interned.
	if(var1->alias.val == var2->alias.val && var1->alias.get_t_id() == var2->alias.get_t_id()){
		if(!vars_same_type_kind(var1, var2)){
			throw std::domain_error(
				fmt::format("Different types or kinds for Var instances with same alias in expression. "
						    "Cannot reconcile {} and {}.", var1->repr(), var2->repr())
			);
		}
		return true;
	}
	return false;
}

bool bases_semantically_equal(Var* var1, Var* var2){
	return vars_semantically_equal(var1->base, var2->base);
}

bool SemanticVarPtr::operator==(const SemanticVarPtr& other) const { 
	return vars_semantically_equal(var_ptr, other.var_ptr);
}

bool SemanticVarPtr::operator<(const SemanticVarPtr& other) const{
	if(uint64_t(var_ptr->alias.val) == uint64_t(other.var_ptr->alias.val)){
		if(!vars_same_type_kind(var_ptr, other.var_ptr)){
			throw std::domain_error(
				fmt::format("Different types or kinds for Var instances with same alias in expression. "
							"Cannot reconcile {} and {}.", var_ptr->repr(), other.var_ptr->repr())
			);
		}
	}
	return uint64_t(var_ptr->alias.val) < uint64_t(other.var_ptr->alias.val);
}

bool Var::operator==(const Var& other) const {
	if(uint64_t(base) != uint64_t(other.base)) return false;
	if(length != other.length) return false;

	for(size_t i=0; i < length; i++){
		DerefInfo& dia = this->deref_infos[i];
		DerefInfo& dib = other.deref_infos[i];

		if(dia.deref_type != dib.deref_type || 
		   dia.mbr_ind != dib.mbr_ind ||
		   dia.deref_kind != dib.deref_kind){
			return false;
		}
	}
	return true;
};


/* FNV-1a */
uint64_t CREHash::operator()(Var* var){
	// if(var->hash != 0){
    //     return var->hash;
    // }   


	uint64_t constexpr fnv_prime = 1099511628211ULL;
  	uint64_t constexpr fnv_offset_basis = 14695981039346656037ULL;


  	// cout << "hash start var: " << var << endl;
 	uint64_t hash = fnv_offset_basis; //^ (end-start * fnv_prime);
  	hash = hash ^ CREHash{}(var->alias);
  	hash = hash * fnv_prime;

  	// cout << "\nMOO:" << hash << ", " << fnv_offset_basis << endl;

  	for(size_t i=0; i < var->size(); ++i){
  		const DerefInfo& di = var->deref_infos[i];

  		// cout << "i: " << i << endl;
      	// Item& item = *x->get(i);
      	// cout << "item: " << item << endl;
      	// uint64_t item_hash = CREHash{}(item);

      	uint32_t u = (uint32_t(di.mbr_ind) << 16) | uint32_t(di.deref_kind);
      	// cout << i << " U:" << u << " " << di.mbr_ind << " " << di.deref_kind <<  endl;
      	hash = hash ^ u;
      	hash = hash * fnv_prime;
      	
  }
  var->hash = hash;
  return hash; 
}

void (*ext_locate_var_alias)(Var*) = nullptr;


// void EnsureVarsNamed(const std::vector<Var*>& vars){
// 	HashSet<std::string_view> alias_map = {};
// 	for(auto var : vars){
// 		if(var->alias != ""){
// 			alias_map.insert(var->alias);
// 		}
// 	}
// }


} // NAMESPACE_END(cre)
