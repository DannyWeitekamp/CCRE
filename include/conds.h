#pragma once

#include <cstddef>
#include <vector>
#include <iosfwd>                // for ostream
#include <string>                // for string
#include <type_traits>           // for std::is_same_v, std::remove_pointer_t
#include <map>                   // for std::map
#include "../include/cre_obj.h"  // for CRE_Obj
#include "../include/ref.h"      // for ref
#include "../include/literal.h"  // for Literal, new_literal
#include "../include/t_ids.h"    // for T_ID_CONDITIONS


// Forward declarations
namespace cre {
struct AllocBuffer;
struct Func;
struct Var;
}

namespace cre {


const uint8_t CONDS_KIND_AND = 1;
const uint8_t CONDS_KIND_OR = 2;

struct VarInfo {
    Var* var;
    size_t pos=-1;
    size_t kind_pos=-1;
    size_t first_item=-1;
    std::vector<size_t> item_inds = {};
    uint8_t kind;

    VarInfo(Var* var, uint8_t kind, size_t first_item) :
        var(var), kind(kind), first_item(first_item) {}
};


typedef std::map<Var*, VarInfo> VarMapType;

struct Logic : public CRE_Obj {
	static constexpr uint16_t T_ID = T_ID_LOGIC;

	// -- Members --
	std::vector<ref<CRE_Obj>> items = {};
    VarMapType var_map = {};
    std::vector<Var*> vars = {};
    std::vector<size_t> standard_order = {};
    std::vector<std::tuple<size_t, size_t>> standard_var_spans = {};
    std::vector<VarMapType::iterator> var_map_iters = {};

    size_t n_abs_vars = 0;
    // Absolute Vars: Regular Vars that must be bound in a match to the Logic pattern.
    // std::vector<Var*> abs_vars = {};
    // Bound Vars: Vars that are effectively an alias for another Var 
    // std::vector<Var*> bnd_vars = {};
    // Optional Vars: Vars that are not explicitly Existential 
    //   but not gaurenteed to be bound in a valid match, usually
    //   because they contribute to a disjunct.
    // std::vector<Var*> opt_vars = {};
    // Existential Vars: Exist() or Not() Vars 
    // std::vector<Var*> ext_vars = {};

	uint8_t kind;  // CONDS_KIND_AND or CONDS_KIND_OR
    uint8_t is_pure_conj;  

	// -- Methods --
	Logic(uint8_t kind);


    void _insert_literal(ref<Literal> lit);
    void _insert_var(Var* var, bool part_of_item=false, uint8_t kind=uint8_t(-1));
    void _insert_arg(CRE_Obj* obj);
    void _extend_same_kind(ref<Logic> conj);
    void _finalize();

    std::string basic_str();
    std::string standard_str(std::string_view indent="  ", 
                             HashSet<Var*>* = nullptr);
    std::string to_string();
    void _ensure_standard_order();

    template<typename... Args>
    void _populate(Args&&... args) {
        (_insert_arg((CRE_Obj*) args), ...);
        _finalize();
    }

    
};

ref<Logic> new_logic(uint8_t kind, AllocBuffer* buffer = nullptr);

std::ostream& operator<<(std::ostream& out, Logic* logic);
std::ostream& operator<<(std::ostream& out, ref<Logic> logic);




template<typename... Args>
ref<Logic> AND(Args&&... args) {
    ref<Logic> logic = new_logic(CONDS_KIND_AND);
    logic->_populate(std::forward<Args>(args)...);
    return logic;
}

template<typename... Args>
ref<Logic> OR(Args&&... args) {
    ref<Logic> logic = new_logic(CONDS_KIND_OR);
    logic->_populate(std::forward<Args>(args)...);
    return logic;
}


} // NAMESPACE_END(cre)

