#include "../include/types.h"
#include "../include/cre_obj.h"

const uint16_t DEREF_KIND_ATTR = 1;
const uint16_t DEREF_KIND_LIST = 2 ;


struct DerefInfo {
	CRE_Type* deref_type; 
	int16_t a_id;
	uint16_t deref_kind;
};


struct Var : public CRE_Obj{
	// -- Members --
// The pointer of the Var instance before any attribute selection
//   e.g. if '''v = Var(Type); v_b = v.B;''' then v_b.base = &v
	Var* base;

// The type of the Var's base instance
	CRE_Type* base_type;

// The type that the Var resolves to
	CRE_Type* head_type;

// The name of the var (uniqueness is not enforced by name)
	std::string_view alias;

// Instructions for applying each dereference 
//   e.g. v.B.A[0] is three dereferences
	DerefInfo* deref_infos;
	size_t length; // Note: could be smaller than size_t

// -- Methods --
	Var* extend_attr(std::string_view attr);
	// uint8_t is_not;

	Item* apply_deref(CRE_Obj* obj);
	// -- Methods --
};

Var* new_var(CRE_Type* _type,
 			std::string_view _alias,
 			DerefInfo* deref_infos=NULL,
 			size_t length=0);

std::string var_to_string(Var* var);
std::ostream& operator<<(std::ostream& out, Var* var);

extern "C" Item* deref_once(CRE_Obj* obj, const DerefInfo& inf);
extern "C" Item* deref_multiple(CRE_Obj* obj, DerefInfo* deref_infos, size_t length);
