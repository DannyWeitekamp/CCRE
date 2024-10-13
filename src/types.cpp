// #include "../include/cre.h"
#include "../include/types.h"
#include "../include/hash.h"
#include "../include/context.h"
#include <iostream>
#include <bitset>

// using namespace std;

// Type constructor implementation
CRE_Type::CRE_Type(std::string_view _name, 
           uint16_t _t_id,
           std::vector<CRE_Type*> _sub_types, 
           uint8_t _builtin)
    : name(std::string(_name)), t_id(_t_id), sub_types(_sub_types), builtin(_builtin) {}


inline uint8_t get_flag_width(uint8_t flag){
    switch (flag){
        case BIFLG_UNIQUE_ID:
            return 1;
        case BIFLG_VISIBLE_ID:
            return 1;
        case BIFLG_SEMANTIC_ID:
            return 1;
        case BIFLG_VERBOSITY:
            return 8;
        default:
            return 0;
    }   
}

void set_flag(MemberSpec* ms, uint64_t flag_n, uint64_t val) {
    uint64_t width = get_flag_width(flag_n); // should inline const
    uint64_t val_mask = ((1 << width) - 1);
    ms->builtin_flags = (ms->builtin_flags & ~uint64_t(val_mask << flag_n)) |
                        uint64_t((val & val_mask) << flag_n);

    // std::bitset<10> btmp(val_mask);
    // std::bitset<10> tmp(ms->builtin_flags);    
    // std::cout << "SET FLAG(" << tmp  << ", " << int(flag_n) << ", width=" << width << ", " << "mask=" << btmp << ")" << std::endl;
}

uint64_t get_flag(MemberSpec* ms, uint64_t flag_n) {
    uint64_t width = get_flag_width(flag_n); // should inline const
    uint64_t val_mask = ((1 << width) - 1) << flag_n;
    uint64_t out = (val_mask & ms->builtin_flags) >> (flag_n);


    // std::bitset<10> btmp(val_mask);    
    // std::bitset<10> tmp(ms->builtin_flags);    
    // std::cout << "GET FLAG(" << tmp << ", " << int(flag_n) << ", width=" << width << ", " << "mask=" << btmp << ")=" << out << std::endl;
    return out;
}

void _MemberSpec_init_flags(MemberSpec* ms, HashMap<std::string, Item> flags){
    ms->builtin_flags = 0;

    // Default Flag Values
    set_flag(ms, BIFLG_UNIQUE_ID, 0);
    set_flag(ms, BIFLG_VISIBLE_ID, 1);
    set_flag(ms, BIFLG_SEMANTIC_ID, 0);
    set_flag(ms, BIFLG_VERBOSITY, 1);

    for (auto& [key, value] : flags){
        // All builtin flags are boolean
        if(value.t_id != T_ID_BOOL and value.t_id != T_ID_INT){
            continue;
        }
        // bool is_set = item_get_bool(value);
        uint64_t val = item_get_int(value);
        if(key == "unique_id")
            set_flag(ms, BIFLG_UNIQUE_ID, val);
        if(key == "visible")
            set_flag(ms, BIFLG_VISIBLE_ID, val);
        if(key == "semantic")
            set_flag(ms, BIFLG_SEMANTIC_ID, val);
        if(key == "verbosity")
            set_flag(ms, BIFLG_VERBOSITY, val);
    }
}

MemberSpec::MemberSpec(std::string_view _name,
             CRE_Type* _type,
             HashMap<std::string, Item> _flags) :
    name(_name), type(_type), flags(_flags){
    _MemberSpec_init_flags(this, _flags);
}

uint64_t MemberSpec::get_flag(uint64_t flag){
    return ::get_flag(this, flag);
}


FactType::FactType(std::string_view _name, 
           std::vector<CRE_Type*> _sub_types, 
           std::vector<MemberSpec> _members,
           HashMap<std::string, Item> _flags)
    : CRE_Type(_name, T_ID_FACT, _sub_types,  0), members(_members), flags(_flags) {

    }




// Operator<< implementation
std::ostream& operator<<(std::ostream& outs, const FactType& t) {
    return outs << t.name;
}

// to_string implementation
std::string to_string(const FactType& value) {
    std::ostringstream ss;
    ss << value;
    return ss.str();
}


// define_type implementation
CRE_Type* define_type(std::string_view name,
                  uint16_t t_id, 
                  std::vector<CRE_Type*> sub_types,
                  CRE_Context* context) {
    if(context == nullptr){
        context = default_context;
    };
    CRE_Type* t = new CRE_Type(name, t_id, sub_types, 1);
    // cout << "DEFINE TYPE" << t->name << endl;
    uint16_t index = context->_add_type(t);
    return context->types[t_id];
}

FactType* define_fact(std::string_view name, 
                  std::vector<MemberSpec> members,
                  std::vector<CRE_Type*> sub_types,
                  HashMap<std::string, Item> flags,
                  CRE_Context* context) {
    if(context == nullptr){
        context = default_context;
    };
    // cout << "DEFINE FACT0: " << name << endl;
    FactType* t = new FactType(name, sub_types, members, flags);
    // cout << "DEFINE FACT" << t->name << endl;
    uint16_t t_id = context->_add_type(t);
    return (FactType*) context->types[t_id];
}

size_t FactType::get_index(std::string_view key){
    for(size_t i = 0; i < this->members.size(); i++){
        if(this->members[i].name == key){
            return i;
        }
    }
    return -1;
}

extern "C" size_t FactType_get_member_index(FactType* type, char* key){
    std::string_view key_view(key);
    return type->get_index(key_view);
}




// Global variable definitions
CRE_Type* cre_undefined;// = new CRE_Type("undefined",{}, 1);
CRE_Type* cre_bool;// = new CRE_Type("bool",{}, 1);
CRE_Type* cre_int;// = new CRE_Type("int",{}, 1);
CRE_Type* cre_float;// = new CRE_Type("float",{}, 1);
CRE_Type* cre_str;// = new CRE_Type("str",{}, 1);
CRE_Type* cre_obj;// = new CRE_Type("obj",{}, 1);
CRE_Type* cre_Fact;// = new CRE_Type("Fact",{}, 1);
CRE_Type* cre_FactSet;// = new CRE_Type("Fact",{}, 1);
CRE_Type* cre_Var;// = new CRE_Type("Var",{}, 1);
CRE_Type* cre_Func;// = new CRE_Type("Func",{}, 1);
CRE_Type* cre_Literal;// = new CRE_Type("Literal",{}, 1);
CRE_Type* cre_Conditions;// = new CRE_Type("Conditions",{}, 1);
CRE_Type* cre_Rule;// = new CRE_Type("Rule",{}, 1);


std::vector<CRE_Type*> cre_builtins;// = {};

void ensure_builtins(){
    if(cre_builtins.size() == 0){
        cre_undefined = new CRE_Type("undefined", T_ID_UNDEFINED, {}, 1);
        cre_bool = new CRE_Type("bool", T_ID_BOOL, {}, 1);
        cre_int = new CRE_Type("int", T_ID_INT, {}, 1);
        cre_float = new CRE_Type("float", T_ID_FLOAT, {}, 1);
        cre_str = new CRE_Type("str", T_ID_STR, {}, 1);
        cre_obj = new CRE_Type("obj", T_ID_OBJ, {}, 1);
        cre_Fact = new CRE_Type("Fact", T_ID_FACT, {}, 1);
        cre_FactSet = new CRE_Type("FactSet", T_ID_FACTSET, {}, 1);
        cre_Var = new CRE_Type("Var", T_ID_VAR, {}, 1);
        cre_Func = new CRE_Type("Func", T_ID_FUNC, {}, 1);
        cre_Literal = new CRE_Type("Literal", T_ID_LITERAL, {}, 1);
        cre_Conditions = new CRE_Type("Conditions", T_ID_CONDITIONS, {}, 1);
        cre_Rule = new CRE_Type("Rule", T_ID_RULE, {}, 1);

        // cout << "INIT builtins";
        cre_builtins = {
            cre_undefined,
            cre_bool,
            cre_int,
            cre_float,
            cre_str,
            cre_obj,
            cre_Fact,
            cre_FactSet,
            cre_Var,
            cre_Func,
            cre_Literal,
            cre_Conditions,
            cre_Rule,
        };    
    }
}

