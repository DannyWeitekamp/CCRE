#include "../include/var.h"
#include "../include/intern.h"
#include "../include/fact.h"
#include <fmt/format.h>
#include <fmt/ranges.h>


// extern "C" 

void _init_var(Var* var,
			CRE_Type* _type,
 			std::string_view _alias,
 			DerefInfo* _deref_infos,
 			size_t _length,
 			size_t copy_len=-1){
	var->alias = intern(_alias);
	var->deref_infos = ((DerefInfo*) ((char*) var) + sizeof(Var));
	var->length = _length;
	var->base_type = _type;

	copy_len = (copy_len == -1) ? _length : copy_len;
	// cout << "INIT: " << uint64_t(copy_len) << ", " <<  uint64_t(_length) << endl; 
	if(_length == 0 || copy_len < _length){
		var->head_type = _type;
	}else{
		var->head_type = _deref_infos[copy_len-1].deref_type;
		memcpy(var->deref_infos, _deref_infos, copy_len*sizeof(DerefInfo));
	}
	// cout << "BASE: " << uint64_t(var->base_type) << endl; 	
	// cout << "HEAD: " << uint64_t(var->head_type) << endl; 	
}

Var* new_var(CRE_Type* _type,
 			std::string_view _alias,
 			DerefInfo* _deref_infos,
 			size_t _length){
	if(_type == nullptr){
		throw std::invalid_argument("Cannot initialize Var from NULL type.");
	}

	// Allocate a new var 
	Var* var = (Var*) malloc(sizeof(Var) + _length*sizeof(DerefInfo)); 
	_init_var(var, _type, _alias, _deref_infos, _length);
	return var;
}

// Var* _alloc_extension(Var* var, size_t extend_by){
// 	Var* new_var = (Var*) malloc(sizeof(Var) + (var->length + extend_by)*sizeof(DerefInfo)); 
// 	memcpy(new_var, var, sizeof(Var) + var->length*sizeof(DerefInfo));
// 	return new_var;
// }

extern "C" Var* Var_extend_attr(Var* var, std::string_view attr){

	// cout << "EXTEND: " << endl;
	if(var->head_type->builtin){
		throw std::runtime_error(
			"Attempting to extend Var " + var_to_string(var) + \
			 " with builtin head type " + var->head_type->name + ".");
	}
	// Allocate a new var 
	// Var* new_var = _alloc_extension(var, 1);
	Var* nv = new_var(var->base_type, var->alias, var->deref_infos, var->length+1);
	// memcpy(nv, var, sizeof(Var) + var->length*sizeof(DerefInfo));
	DerefInfo* new_deref_inf = &nv->deref_infos[var->length];

	FactType* head_type = (FactType*) var->head_type;

	// cout << "head_type: " << uint64_t(head_type) << " " << int(head_type->kind) << endl;
	CRE_Type* deref_type = head_type->get_attr_type(attr);
	int a_id = head_type->get_attr_index(attr);
	new_deref_inf->deref_type = deref_type;
	new_deref_inf->a_id = a_id;
	new_deref_inf->deref_kind = DEREF_KIND_ATTR;
	nv->head_type = deref_type;
	// _init_var(var, _type, _alias, _deref_infos, _length+1);
	return nv;
}

Var* Var::extend_attr(std::string_view attr){
	return Var_extend_attr(this, attr);
}

std::string var_to_string(Var* var){
	vector<string> deref_strs = {};
	deref_strs.reserve(var->length);

	// std::string out = std::string(var.alias);
	CRE_Type* type = var->base_type;
	for(int i=0; i < var->length; i++){
		FactType* fact_type = (FactType*) type;
		DerefInfo inf = var->deref_infos[i];
		int a_id = int(inf.a_id);
		
		if(inf.deref_kind == DEREF_KIND_ATTR){
			// cout << "a_id=" << int(a_id) <<
			// 	"attr=" << fact_type->get_item_attr(a_id) <<
			//  	endl;
			deref_strs.emplace_back(
				fmt::format(".{}", fact_type->get_item_attr(a_id))
			);
		}else if(inf.deref_kind == DEREF_KIND_LIST){
			deref_strs.emplace_back(
				fmt::format("[{}]", a_id)
			);
		}
		type = fact_type->get_item_type(a_id);	
	}
	return fmt::format("{}{}", var->alias, fmt::join(deref_strs, ""));
}

std::ostream& operator<<(std::ostream& out, Var* var){
	return out << var_to_string(var);
}

extern "C" Item* deref_once(CRE_Obj* obj, const DerefInfo& inf){
	switch(inf.deref_kind){

	case DEREF_KIND_ATTR:
		{
		Fact* fact = (Fact*) obj;
		return fact->get(inf.a_id);
		}
	case DEREF_KIND_LIST:
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
