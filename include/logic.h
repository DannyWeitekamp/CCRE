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
#include "../include/hash.h"   // for HashSet
#include "../include/var_inds.h" // for VarInds

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

const uint8_t LIT_SEMANTICS_REG = 1;
const uint8_t LIT_SEMANTICS_FACT = 2;
const uint8_t LIT_SEMANTICS_OR_CONSTS = 3;

struct LiteralSemantics {
    
    size_t first_item=-1;
    size_t last_item=-1;
    uint8_t kind;

    LiteralSemantics() : kind(LIT_SEMANTICS_REG){}
    LiteralSemantics(uint8_t kind, size_t first_item, size_t last_item) :
    kind(kind), first_item(first_item), last_item(last_item) {}
};





typedef std::map<SemanticVarPtr, VarInfo> VarMapType;

struct Logic : public CRE_Obj {
	static constexpr uint16_t T_ID = T_ID_LOGIC;

	// -- Members --
	std::vector<Item> items = {};
    VarMapType var_map = {};
    std::vector<ref<Var>> vars = {};
    std::vector<size_t> standard_order = {};
    // std::vector<std::tuple<size_t, size_t>> standard_var_spans = {};
    std::vector<size_t> const_item_inds = {};
    std::vector<VarMapType::iterator> var_map_iters = {};
    std::vector<LiteralSemantics> lit_semantics = {};

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
    uint8_t is_pure_conj = true;  
    uint8_t is_pure_disj = true;
    uint8_t is_pure_const_or = true;  

	// -- Methods --
	Logic(uint8_t kind);
    void _insert_const(const Item& arg);
    void _insert_arg(const Item& arg, LiteralSemantics semantics=LiteralSemantics());
    void _insert_var(Var* var, bool part_of_item=false, uint8_t kind=uint8_t(-1));
    void _insert_literal(ref<Literal> lit, const LiteralSemantics& semantics=LiteralSemantics());
    // void _insert_fact_as_literals(ref<Fact> fact);
    void _insert_other_kind(ref<Logic> logic);
    void _extend_same_kind(ref<Logic> conj);
    void _finalize();

std::string basic_str();
    size_t _stream_item(std::stringstream& ss, size_t i, std::string_view indent,
         HashSet<void*>* var_covered, std::vector<bool>& item_covered, size_t& n_items_covered, bool is_first=false);
    std::string standard_str(std::string_view indent="  ", 
        std::string_view prev_indent="", HashSet<void*>* var_covered = nullptr);
    std::string to_string();
    void _ensure_standard_order();

    template<typename... Args>
    void _populate(Args&&... args) {
        (_insert_arg( Item(args)), ...);
        _finalize();
    }

    
};


ref<Logic> new_logic(uint8_t kind, AllocBuffer* buffer = nullptr);
ref<Logic> fact_to_conjunct(Fact* fact, Var* var=nullptr, AllocBuffer* alloc_buffer=nullptr);
ref<Logic> distribute_OR_const(Logic* disj, Var* var, AllocBuffer* alloc_buffer=nullptr);

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

