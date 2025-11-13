#pragma once

#include <cstdint>
#include <stddef.h>              // for size_t, NULL
#include <stdint.h>              // for uint16_t, int16_t, uint32_t, uint64_t
#include <iosfwd>                // for ostream
#include <string>                // for string
#include <string_view>           // for string_view
#include <vector>                // for vector
#include "../include/cre_obj.h"  // for CRE_Obj
#include "../include/intern.h"   // for InternStr
#include "../include/ref.h"      // for ref
#include "item.h"                // for Item
#include "t_ids.h"               // for T_ID_VAR
namespace cre { struct AllocBuffer; }
namespace cre { struct CRE_Type; }
namespace cre { struct Member; }


namespace cre {


const uint8_t VAR_KIND_ABSOLUTE = 1;
const uint8_t VAR_KIND_BOUND = 2;
const uint8_t VAR_KIND_OPTIONAL = 3; 
const uint8_t VAR_KIND_EXIST = 4;
const uint8_t VAR_KIND_NOT = 5;
// const uint8_t VAR_KIND_ALL = 4;

const std::string VAR_PREFIXES[6] = {"","Var", "Bound", "Opt", "Exist", "Not"};


const uint16_t DEREF_KIND_ATTR = 1;
const uint16_t DEREF_KIND_ITEM = 2 ;



struct DerefInfo {
	CRE_Type* deref_type; 
	int16_t mbr_ind;
	uint16_t deref_kind;
	uint32_t pad;
};


struct Var : public CRE_Obj{
	static constexpr uint16_t T_ID = T_ID_VAR;

	// -- Members --
// The pointer of the Var instance before any attribute selection
//   e.g. if '''v = Var(Type); v_b = v.B;''' then v_b.base = &v
	// Note: we don't use ref<Var> so that Var is POD 
	//   so that free() makes sense. Thus we need to explicitly 
	Var* base; 

// The type of the Var's base instance
	CRE_Type* base_type;

// The type that the Var resolves to
	CRE_Type* head_type;

// The name of the var (uniqueness is not enforced by name)
	Item alias;

// Instructions for applying each dereference 
//   e.g. v.B.A[0] is three dereferences
	DerefInfo* deref_infos;
	size_t length; // Note: could be smaller than size_t

	Item bound_obj = Item();

	uint64_t hash;

	uint8_t kind=false;

// -- Methods --
	Var(const Item& _alias,
		CRE_Type* _type=nullptr,
		uint8_t kind = VAR_KIND_ABSOLUTE,
		DerefInfo* _deref_infos=nullptr,
		size_t _length=0);

	// Var(const std::string_view& _alias,
	// 	CRE_Type* _type=nullptr,
	// 	DerefInfo* _deref_infos=nullptr,
	// 	size_t _length=0);

	// ref<Var> _extend_unsafe(int mbr_ind, uint16_t deref_kind, AllocBuffer* alloc_buffer=nullptr);
	ref<Var> _extend_unsafe(DerefInfo* derefs, size_t n_derefs, AllocBuffer* alloc_buffer=nullptr);
	ref<Var> extend_attr(const std::string_view& attr, AllocBuffer* alloc_buffer=nullptr);
	ref<Var> extend_item(int16_t mbr_ind, 			   AllocBuffer* alloc_buffer=nullptr);

	void swap_base(Var* new_base);
	// uint8_t is_not;

	Item* apply_deref(CRE_Obj* obj);

	inline size_t size(){
		return length;
	}
	inline size_t is_existential(){
		return kind > VAR_KIND_EXIST;
	}

	inline CRE_Type* eval_type() const {
		return head_type;
	}
	inline uint16_t eval_t_id() const {
		return head_type->get_t_id();
	}
	// inline size_t is_universal(){
	// 	return kind & 4;
	// }
	std::string get_alias_str();
	std::string get_deref_str() ;
	std::string get_prefix_str();
	std::string to_string();
	std::string repr(bool use_alias=true) ;
	bool operator==(const Var& other) const;

};





ref<Var> new_var(const Item& alias,
 			CRE_Type* type=nullptr,
 			uint8_t kind=VAR_KIND_ABSOLUTE,
 			DerefInfo* deref_infos=NULL,
 			size_t length=0,
 			AllocBuffer* buffer=nullptr);


ref<Var> Not(const Item& alias, CRE_Type* type=nullptr, DerefInfo* deref_infos=NULL, size_t length=0, AllocBuffer* buffer=nullptr);
ref<Var> Exists(const Item& alias, CRE_Type* type=nullptr, DerefInfo* deref_infos=NULL, size_t length=0, AllocBuffer* buffer=nullptr);
ref<Var> Bound(const Item& alias, CRE_Type* type=nullptr, DerefInfo* deref_infos=NULL, size_t length=0, AllocBuffer* buffer=nullptr);
ref<Var> Opt(const Item& alias, CRE_Type* type=nullptr, DerefInfo* deref_infos=NULL, size_t length=0, AllocBuffer* buffer=nullptr);

bool vars_same_type_kind(Var* var1, Var* var2);
bool bases_semantically_equal(Var* var1, Var* var2);
bool vars_semantically_equal(Var* var1, Var* var2);
// ref<Var> new_Exists(const Item& alias,
//  			CRE_Type* type=nullptr,
//  			DerefInfo* deref_infos=NULL,
//  			size_t length=0,
//  			AllocBuffer* buffer=nullptr);

// ref<Var> new_Not(const Item& alias,
//  			CRE_Type* type=nullptr,
//  			DerefInfo* deref_infos=NULL,
//  			size_t length=0,
//  			AllocBuffer* buffer=nullptr);

// ref<Var> new_var(const std::string_view& _alias,
//  			CRE_Type* _type=nullptr,
//  			DerefInfo* deref_infos=NULL,
//  			size_t length=0,
//  			AllocBuffer* alloc_buffer=nullptr);


extern void (*ext_locate_var_alias)(Var*);
// std::string var_to_string(Var* var);
std::ostream& operator<<(std::ostream& out, Var* var);
std::ostream& operator<<(std::ostream& out, ref<Var> var);

Member* deref_once(CRE_Obj* obj, const DerefInfo& inf);
Member* deref_multiple(CRE_Obj* obj, DerefInfo* deref_infos, size_t length);



InternStr InventVarName(Var* var, const std::vector<Var*>& other_vars);


// A wrapper over Var* that checks that Vars have the same alias.
//  and throws an error if Vars with the same alias have different types or kinds.
struct SemanticVarPtr {
	Var* var_ptr;

	SemanticVarPtr(Var* _var_ptr) : var_ptr(_var_ptr) {}
	operator Var*() const { return var_ptr; }
	operator ref<Var>() const { return ref<Var>(var_ptr); }

	bool operator==(const SemanticVarPtr& other) const;
	bool operator<(const SemanticVarPtr& other) const;
};

const size_t MAX_UNIQUE_VAR_ALIAS_TRIES = 1000;
template<typename T>
void find_unique_var_alias(Var* var, const T& other_vars){
	size_t ind = 0;
	fmt::basic_memory_buffer<char, 20> buffer;
	char first_letter = toupper(var->base_type->name[0]);

	bool is_unique = false;
	do {
		auto result = fmt::format_to_n(buffer.data(), buffer.capacity(),
		 				 "{}{}", first_letter, ind);
		var->alias = Item(intern(std::string_view(buffer.data(), result.size)));
		++ind;
		auto it = other_vars.find(SemanticVarPtr(var));

		is_unique = (it == other_vars.end());
	}while (!is_unique && ind < MAX_UNIQUE_VAR_ALIAS_TRIES);

	if(ind >= MAX_UNIQUE_VAR_ALIAS_TRIES){
		throw std::runtime_error(
		"A logical expression failed to generate a unique variable alias. "
		"This can happen if the expression contains a huge number of variables "
		"(>4.29 trillon) and no alias was explicitly provided for at least one of them."
	);
	}
}



#define SIZEOF_VAR(n) (sizeof(Var)+(n)*sizeof(DerefInfo))

} // NAMESPACE_END(cre)
