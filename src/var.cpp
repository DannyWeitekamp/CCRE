#include "../include/var.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include "../include/ref.h"
#include <fmt/format.h>
#include <fmt/ranges.h>


// extern "C" 

void Var_dtor(const CRE_Obj* x){
	Var* var = (Var*) x;

	if(var->base != 0 && var->base != var){
		var->base->dec_ref();
	}

	if(x->alloc_buffer == nullptr){
    	free((void*) x);
	}else{
		// NOTE: We need to do this because cannot
		//  write alloc_buffer as a ref<AllocBuffer> 
		x->alloc_buffer->dec_ref();
	}
}


Var::Var(const Item& _alias,
 		CRE_Type* _type,
 		DerefInfo* _deref_infos,
 		size_t _length) : 

		CRE_Obj(&Var_dtor),
		base(nullptr), 
		base_type(_type), head_type(_type),
		alias(_alias),
		deref_infos(nullptr),
		length(_length),
		hash(0){


	// if(_type == nullptr){
	// 	throw std::invalid_argument("Cannot initialize Var from NULL type.");
	// }

	deref_infos = ((DerefInfo*) (( (char*) this) + sizeof(Var)) );

	// cout << "deref_infos_offset=" << uint64_t(deref_infos) - uint64_t(this) << endl;

	if(_deref_infos != nullptr && _length > 0){
		head_type = _deref_infos[_length-1].deref_type;
		memcpy(deref_infos, _deref_infos, _length*sizeof(DerefInfo));
	}

	if(_alias.t_id != T_ID_STR && _alias.t_id != T_ID_INT){
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
			const Item& _alias,
			CRE_Type* _type,
 			DerefInfo* _deref_infos,
 			size_t _length,
 			AllocBuffer* alloc_buffer){
	

	bool did_malloc = true;
	Var* var;
	if(alloc_buffer != nullptr){
		var = (Var*) alloc_buffer->alloc_bytes(SIZEOF_VAR(_length), did_malloc);	
	}else{
		var = (Var*) malloc(SIZEOF_VAR(_length)); 
	}
    
    var = new (var) Var(_alias, _type, _deref_infos, _length);

    if(!did_malloc){
    	var->alloc_buffer = alloc_buffer;
    	alloc_buffer->inc_ref();
    }
    
    var->hash = CREHash{}(var);
	// Allocate a new var 
	
	// _init_var(var, _type, _alias, _deref_infos, _length);
	// var->dtor = Var_dtor;
	return var;
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

ref<Var> Var::_extend_unsafe(int mbr_ind, uint16_t deref_kind, AllocBuffer* alloc_buffer){
	// Allocate a new var 
	// Var* new_var = _alloc_extension(var, 1);
	Var* __restrict nv;
	bool did_malloc = true;

	// cout << "SIZEOF VAR" << sizeof(Var) << ", " << SIZEOF_VAR(length+1) << endl;
	if(alloc_buffer != nullptr){
		nv = (Var*) alloc_buffer->alloc_bytes(SIZEOF_VAR(length+1), did_malloc);	
	}else{
		nv = (Var*) malloc(SIZEOF_VAR(length+1)); 
	}

	nv = new (nv) Var(alias, base_type, nullptr, length+1);

	if(!did_malloc){
		nv->alloc_buffer = alloc_buffer;
		alloc_buffer->inc_ref();
	}

	// Var* nv = new_var(base_type, alias, deref_infos, length+1);
	// cout << "size=" << sizeof(Var) << " d_infs=" << uint64_t(nv->deref_infos)-uint64_t(nv) 
	// 	 << " end=" << SIZEOF_VAR(length+1) << " [len]= " << uint64_t(&nv->deref_infos[length])-uint64_t(nv) << endl;
	// for(uint i=0; i < length; i++){
	// 	nv->deref_infos[i] = deref_infos[i]; 		
	// }

	memcpy(nv->deref_infos, deref_infos, length*sizeof(DerefInfo));
	DerefInfo* __restrict new_deref_inf = &nv->deref_infos[length];

	FactType* hf_type = (FactType*) head_type;
	

	// cout << "head_type: " << uint64_t(hf_type) << " " << int(hf_type->kind) << endl;
	// Set the trailing deref_info

	
	CRE_Type* deref_type = hf_type->get_item_type(mbr_ind);
	
	new_deref_inf->deref_type = deref_type;
	new_deref_inf->mbr_ind = mbr_ind;
	new_deref_inf->deref_kind = deref_kind;

	// Modify the new var 

	nv->base = this->base; this->base->inc_ref();
	nv->head_type = deref_type;
	nv->length = length+1;
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
	return _extend_unsafe(mbr_ind, DEREF_KIND_ATTR, alloc_buffer);
}

ref<Var> Var::extend_item(int16_t mbr_ind, AllocBuffer* alloc_buffer){
	return _extend_unsafe(mbr_ind, DEREF_KIND_ITEM, alloc_buffer);	
}



std::string Var::to_string(){
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
	if(alias.t_id == T_ID_STR){
		return fmt::format("{}{}", alias.as_string(), fmt::join(deref_strs, ""));	
	}else if(alias.t_id == T_ID_INT){
		return fmt::format("F{}{}", alias.as_int(), fmt::join(deref_strs, ""));	
	}
	
}

std::ostream& operator<<(std::ostream& out, Var* var){
	return out << var->to_string();
}

extern "C" Item* deref_once(CRE_Obj* obj, const DerefInfo& inf){
	switch(inf.deref_kind){

	case DEREF_KIND_ATTR:
		{
		Fact* fact = (Fact*) obj;
		return fact->get(inf.mbr_ind);
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

extern "C" Item* deref_multiple(CRE_Obj* obj, DerefInfo* deref_infos, size_t length){
	Item* item_ptr = nullptr;
	for(int i=0; i < length; i++){
		item_ptr = deref_once(obj, deref_infos[i]);
		if(item_ptr == nullptr) break;
		obj = (CRE_Obj*) item_ptr->val;
	}
	return item_ptr;
}


Item* Var::apply_deref(CRE_Obj* obj){
	return deref_multiple(obj, this->deref_infos, this->length);
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
