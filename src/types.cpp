// #include "../include/cre.h"
#include "../include/types.h"
#include "../include/hash.h"
#include "../include/context.h"
#include <iostream>
#include <bitset>
#include <fmt/format.h>

// using namespace std;

// Type constructor implementation
CRE_Type::CRE_Type(
           std::string_view _name, 
           uint16_t _t_id,
           std::vector<CRE_Type*> _sub_types, 
           uint8_t _builtin) : 
    name(std::string(_name)), t_id(_t_id),
    sub_types(_sub_types), builtin(_builtin),
    context(nullptr), type_index(0) {

    if(builtin){
        this->kind = TYPE_KIND_BUILTIN;
    }
}




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

MemberSpec::MemberSpec(
            std::string_view _name,
            CRE_Type* _type,
            HashMap<std::string, Item> _flags) :
    name(_name), type(_type), flags(_flags){
    _MemberSpec_init_flags(this, _flags);
}

MemberSpec::MemberSpec(
            std::string_view _name,
            std::string_view _type_name,
            HashMap<std::string, Item> _flags) :
    name(_name), flags(_flags){
    CRE_Type* _type  = current_context->get_type(_type_name);
    if(_type == nullptr){
        _type = new DefferedType(_type_name);
    }
    this->type = _type;
    _MemberSpec_init_flags(this, _flags);
}

// CRE_Type* MemberSpec::get_type(){
//     CRE_Type* _type = this->type;
//     if(_type->kind == TYPE_KIND_DEFFERED){
//         DefferedType* dt = (DefferedType*) type;
//         _type  = current_context->get_type(dt->name);
//         this->type = _type;
//     }
//     return _type;
// }

// CRE_Type* MemberSpec::get_type(){
//     return this->type;
//     // CRE_Type* _type = this->type;
//     // if(_type->kind == TYPE_KIND_DEFFERED){
//     //     DefferedType* dt = (DefferedType*) type;
//     //     _type  = current_context->get_type(dt->name);
//     //     this->type = _type;
//     // }
//     // return _type;
// }

bool _try_finalized(FactType* _this, bool do_throw){
    CRE_Context* context = _this->context;
    for(int i=0; i < _this->members.size(); i++){
        MemberSpec* mbr_spec = &_this->members[i];
        CRE_Type* mbr_type = mbr_spec->type;
        if(mbr_type->kind == TYPE_KIND_DEFFERED){
            CRE_Type* resolved_type = context->get_type(mbr_type->name);
            if(resolved_type != nullptr){
                // If found free deffered types and replace
                // std::cout << "RESOLVE TYPE" << std::endl;
                delete mbr_type;
                mbr_spec->type = resolved_type;    
                std::cout << "RESOLVE TYPE: " << uint64_t(resolved_type) << " " << resolved_type << std::endl;
            }else{
                // std::cout << "FAIL RESOLVE TYPE" << std::endl;
                if(!do_throw){
                    return false;
                }
                throw std::runtime_error(
                    "Invalid use of unfinalized FactType '" + _this->name + "'. " + \
                    "Attribute '" + mbr_spec->name + \
                    "' with deffered type '" + mbr_spec->name + \
                    "' has not been defined in the current context: '" + current_context->name + "'" 
                );
            }
        }
    }
    _this->finalized = 1;
    return true;
}

bool FactType::try_finalized(){
    return _try_finalized(this, false);
}

void FactType::ensure_finalized(){
    _try_finalized(this, true);
}

uint64_t MemberSpec::get_flag(uint64_t flag){
    return ::get_flag(this, flag);
}


FactType::FactType(std::string_view _name, 
           std::vector<CRE_Type*> _sub_types, 
           std::vector<MemberSpec> _members,
           HashMap<std::string, Item> _flags)
    : CRE_Type(_name, T_ID_FACT, _sub_types,  0),
      members(_members), flags(_flags) {
    finalized = false;
    kind = TYPE_KIND_FACT;
}




// // Operator<< implementation
// std::ostream& operator<<(std::ostream& outs, const FactType& t) {
//     return outs << t.name;
// }

// // to_string implementation
// std::string to_string(const FactType& value) {
//     std::ostringstream ss;
//     ss << value;
//     return ss.str();
// }


// define_type implementation
CRE_Type* define_type(std::string_view name,
                  uint16_t t_id, 
                  std::vector<CRE_Type*> sub_types,
                  CRE_Context* context) {
    if(context == nullptr){
        context = current_context;
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
        context = current_context;
    };
    // cout << "DEFINE FACT0: " << name << endl;
    FactType* t = new FactType(name, sub_types, members, flags);
    // std::cout << "DEFINE FACT[0]" << t->members[0].builtin_flags << std::endl;
    size_t index = context->_add_type(t);
    // std::cout << uint64_t(t) << " <<KIND" << int(t->kind) << std::endl;
    // std::cout << "<<T_ID" << int(t->t_id) << std::endl;
    t->finalized = t->try_finalized();
    
    return t;//(FactType*) context->types[index];
}

inline void _ensure_index_okay(FactType* type, int index, const std::string& descr){
    if(index >= type->members.size()){
        throw std::runtime_error(descr + "(" + 
            std::to_string(index) + ") failed for type " + type->name + 
            " with " + std::to_string(type->members.size()) + " members." 
        );
    }
}


int FactType::get_attr_index(std::string_view key){
    for(size_t i = 0; i < this->members.size(); i++){
        if(this->members[i].name == key){
            return i;
        }
    }
    return -1;
}

std::string FactType::get_item_attr(int index){
    _ensure_index_okay(this, index, "get_attr_name");
    return this->members[index].name;
}

CRE_Type* FactType::get_item_type(int index){
    _ensure_index_okay(this, index, "get_deref_type");
    return this->members[index].type;
}

CRE_Type* FactType::get_attr_type(std::string_view name){
    int index = this->get_attr_index(name);
    if(index == -1){
        std::cout << uint64_t(this) << " L=" << this->name.length() << std::endl;
        std::cout << "<<" << name << ", " << this->name << std::endl;
        throw std::runtime_error("get_attr_type('" + 
            std::string(name) + "') failed for type " + this->name + 
            " with " + std::to_string(this->members.size()) + " members." 
        );
    }
    return this->members[index].type;
}

extern "C" size_t FactType_get_attr_index(FactType* type, char* key){
    std::string_view key_view(key);
    return type->get_attr_index(key_view);
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

DefferedType::DefferedType(std::string_view _name) :
    CRE_Type(_name, 0, {}, 0) {

    kind = TYPE_KIND_DEFFERED;
}


// to_string implementation
std::string to_string(const CRE_Type* type) {
    if(type->kind == TYPE_KIND_DEFFERED){
        return fmt::format("DefferedType[{}]", type->name);
    }else{
        return std::string(type->name);    
    }
}

std::ostream& operator<<(std::ostream& outs, const CRE_Type* type) {
    return outs << to_string(type);
}


