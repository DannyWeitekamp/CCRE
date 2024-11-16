#include "../include/types.h"
#include "../include/cre_obj.h"
#include "../include/hash.h"
#include "../include/intern.h"
#include "../include/ref.h"

const uint16_t DEREF_KIND_ATTR = 1;
const uint16_t DEREF_KIND_LIST = 2 ;


struct DerefInfo {
	CRE_Type* deref_type; 
	int16_t a_id;
	uint16_t deref_kind;
	uint32_t pad;
};


struct Var : public CRE_Obj{
	// -- Members --
// The pointer of the Var instance before any attribute selection
//   e.g. if '''v = Var(Type); v_b = v.B;''' then v_b.base = &v
	ref<Var> base;

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

// -- Methods --
	Var(const Item& _alias,
		CRE_Type* _type=nullptr,
		DerefInfo* _deref_infos=nullptr,
		size_t _length=0);

	// Var(const std::string_view& _alias,
	// 	CRE_Type* _type=nullptr,
	// 	DerefInfo* _deref_infos=nullptr,
	// 	size_t _length=0);

	ref<Var> extend_attr(const std::string_view& attr, AllocBuffer* alloc_buffer=nullptr);
	ref<Var> _extend_attr_unsafe(int a_id, AllocBuffer* alloc_buffer=nullptr);
	// uint8_t is_not;

	Item* apply_deref(CRE_Obj* obj);

	inline size_t size(){
		return length;
	}
	std::string to_string();

};

ref<Var> new_var(const Item& _alias,
 			CRE_Type* _type=nullptr,
 			DerefInfo* deref_infos=NULL,
 			size_t length=0,
 			AllocBuffer* alloc_buffer=nullptr);

// ref<Var> new_var(const std::string_view& _alias,
//  			CRE_Type* _type=nullptr,
//  			DerefInfo* deref_infos=NULL,
//  			size_t length=0,
//  			AllocBuffer* alloc_buffer=nullptr);



// std::string var_to_string(Var* var);
std::ostream& operator<<(std::ostream& out, Var* var);

extern "C" Item* deref_once(CRE_Obj* obj, const DerefInfo& inf);
extern "C" Item* deref_multiple(CRE_Obj* obj, DerefInfo* deref_infos, size_t length);


#define SIZEOF_VAR(n) (sizeof(Var)+(n)*sizeof(DerefInfo))
