// #include "../include/cre.h"
#include "../include/types.h"
#include "../include/hash.h"
#include "../include/context.h"
#include "../include/item.h"
#include "../include/cre_obj.h"
#include "../include/intern.h"
#include <iostream>
#include <bitset>
#include <fmt/format.h>

using std::cout;
using std::endl;
using std::vector;
namespace cre {


void CRE_Type_dtor(const CRE_Obj* ptr){
    delete ((CRE_Type* ) ptr);
}

// Type constructor implementation
CRE_Type::CRE_Type(
           std::string_view _name, 
           uint16_t _t_id,
           uint16_t _byte_width,
           vector<CRE_Type*> _sub_types, 
           uint8_t _builtin,
           int32_t _type_index, 
           CRE_Context* context) : 
    name(std::string(_name)), t_id(_t_id),
    byte_width(_byte_width), sub_types(_sub_types), builtin(_builtin),
    context(context), type_index(_type_index) {

    // CRE_Obj(&CRE_Type_dtor)
    this->init_control_block(&CRE_Type_dtor);


    if(builtin){
        this->kind = TYPE_KIND_BUILTIN;
    }
}

// CRE_Type::~CRE_Type(){
//     cout << "destroy: " << this << endl;
// }


const FlagGroup default_mbr_flags = 
    FlagGroup({
        {"unique_id", Item(0)},
        {"visible", Item(1)},
        {"semantic", Item(0)},
        {"verbosity", Item(1)}
});


void _MemberSpec_init_flags(
    MemberSpec* ms,
    const HashMap<std::string, Item>& flags){

    // ms->builtin_flags = 0;

    // Default Flag Values
    // set_builtin_flag(&ms->builtin_flags, BIFLG_UNIQUE_ID, 0);
    // set_builtin_flag(&ms->builtin_flags, BIFLG_VISIBLE_ID, 1);
    // set_builtin_flag(&ms->builtin_flags, BIFLG_SEMANTIC_ID, 0);
    // set_builtin_flag(&ms->builtin_flags, BIFLG_VERBOSITY, 1);

    // HashMap<std::string, Item> default_flags= {
    //     {"unique_id", to_item(0)},
    //     {"visible", to_item(1)},
    //     {"semantic", to_item(0)},
    //     {"verbosity", to_item(0)}
    // };
    // ms->flags = FlagGroup(default_flags);
    ms->flags.assign(flags);//FlagGroup(flags);
    // ms->custom_flags = parse_builtin_flags(&ms->builtin_flags, flags);

}

MemberSpec::MemberSpec(
            std::string_view _name,
            CRE_Type* _type,
            const HashMap<std::string, Item>& _flags) :
    name(_name), type(_type), flags(default_mbr_flags){
    _MemberSpec_init_flags(this, _flags);
}

MemberSpec::MemberSpec(
            std::string_view _name,
            std::string_view _type_name,
            const HashMap<std::string, Item>& _flags) :
    name(_name), flags(default_mbr_flags){
    CRE_Type* _type  = current_context->_get_type(_type_name);
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

int get_unique_id_index(const vector<MemberSpec>& member_specs){
    for(int i=0; i < member_specs.size(); i++){
        const MemberSpec& mbr_spec = member_specs[i];
        // cout << "TRY: " << i << " " << mbr_spec.flags.builtin_flags << endl;
        // cout << "<< GET: " << mbr_spec.get_flag(BIFLG_UNIQUE_ID) << endl;
        if(mbr_spec.get_flag(BIFLG_UNIQUE_ID)){
            return i;
        }
    }
    return -1;
}

int get_unique_id_index(FactType* type){
    if(type != nullptr){
        return get_unique_id_index(type->members);
    }
    return -1;
}


bool _try_finalized(FactType* _this, bool do_throw){
    CRE_Context* context = _this->context;
    for(int i=0; i < _this->members.size(); i++){
        MemberSpec* mbr_spec = &_this->members[i];
        CRE_Type* mbr_type = mbr_spec->type;
        if(mbr_type->kind == TYPE_KIND_DEFFERED){
            CRE_Type* resolved_type = context->_get_type(mbr_type->name);
            if(resolved_type != nullptr){
                // If found free deffered types and replace
                // std::cout << "RESOLVE TYPE" << std::endl;
                delete mbr_type;
                mbr_spec->type = resolved_type;    
                // std::cout << "RESOLVE TYPE: " << uint64_t(resolved_type) << " " << resolved_type << std::endl;
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

uint64_t MemberSpec::get_flag(uint64_t flag) const{
    return cre::get_builtin_flag(&this->flags.builtin_flags, flag);
}


void FactType_dtor(const CRE_Obj* ptr){
    // cout << "Type_dtor: " << ((FactType* ) ptr)->name << endl;
    delete ((FactType* ) ptr);
}

FactType::FactType(std::string_view _name, 
           const vector<CRE_Type*>& _sub_types, 
           const vector<MemberSpec>& _members,
           const HashMap<std::string, Item>& _flags,
           CRE_Context* _context)
    : CRE_Type(_name, T_ID_FACT, sizeof(void*), _sub_types,  0, 0, _context), 
      members(_members), flags(_flags) {

    control_block->dtor = &FactType_dtor; 
    finalized = false;
    kind = TYPE_KIND_FACT;


    member_names_as_items = {};
    member_names_as_items.reserve(_members.size());
    for(auto& mbr_spec : _members){

        auto interned_mbr_name = context->intern(mbr_spec.name);
        // auto interned_mbr_name = std::string_view(mbr_spec.name);
        // cout << "INTERN STR:" << interned_mbr_name << endl;
        member_names_as_items.push_back(Item(interned_mbr_name));
    }
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
                  const vector<CRE_Type*>& sub_types,
                  uint16_t byte_width,
                  CRE_Context* context) {
    if(context == nullptr){
        context = current_context;
    };

    // TODO: What is the T_ID of a user defined type
    CRE_Type* t = new CRE_Type(name, 0, byte_width, sub_types, 0, 0, context);
    // cout << "DEFINE TYPE" << t->name << endl;
    uint16_t index = context->_add_type(t);
    t->type_index = index;

    // return context->types[index];
    return t;
}

void _implicit_member_flag_defaults(vector<MemberSpec>& members){

    if(get_unique_id_index(members) == -1){
        // Check for 'id', 'unique_id', '
        for (int i=0; i < members.size(); ++i){
            auto& m_spec = members[i];

            // cout << "m_spec.name: " << m_spec.name << endl; 
            if(m_spec.name == "id" ||
               m_spec.name == "unique_id" ||
               m_spec.name == "uid"){
                m_spec.flags.assign_flag("unique_id", true);
                break;
            }
        }    
    }
}

FactType* define_fact(std::string_view name, 
                  const vector<MemberSpec>& members,
                  const vector<CRE_Type*>& sub_types,
                  const HashMap<std::string, Item>& flags,
                  CRE_Context* context) {
    if(context == nullptr){
        context = current_context;
    };

    // Make a copy
    vector<MemberSpec> members_cpy = members; 
    _implicit_member_flag_defaults(members_cpy);
    // cout << "DEFINE FACT0: " << name << endl;
    FactType* t = new FactType(name, sub_types, members_cpy, flags, context);
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


int FactType::get_attr_index(const std::string_view& attr){
    for(size_t i = 0; i < this->members.size(); i++){
        if(this->members[i].name == attr){
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

CRE_Type* FactType::get_attr_type(const std::string_view& name){
    int index = this->get_attr_index(name);
    if(index == -1){
        // std::cout << uint64_t(this) << " L=" << this->name.length() << std::endl;
        // std::cout << "<<" << name << ", " << this->name << std::endl;
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




vector<CRE_Type*> make_builtins(){
    vector<CRE_Type*> cre_builtins;// = {};

    if(cre_builtins.size() == 0){
        cre_undefined = new CRE_Type("undefined", T_ID_UNDEFINED,
                                0, {}, 1, 0);
        cre_bool = new CRE_Type("bool", T_ID_BOOL,
                                sizeof(bool), {}, 1, 1);
        cre_int = new CRE_Type("int", T_ID_INT,      
                                sizeof(int64_t), {}, 1, 2);
        cre_float = new CRE_Type("float", T_ID_FLOAT,
                                sizeof(double), {}, 1, 3);
        cre_str = new CRE_Type("str", T_ID_STR,      
                                sizeof(std::string), {}, 1 ,4);
        cre_obj = new CRE_Type("obj", T_ID_OBJ,      
                                sizeof(double), {}, 1, 5);
        cre_Fact = new CRE_Type("Fact", T_ID_FACT, 
                                sizeof(void*), {}, 1, 6);
        cre_FactSet = new CRE_Type("FactSet", T_ID_FACTSET,
                                sizeof(void*), {}, 1, 7);
        cre_Var = new CRE_Type("Var", T_ID_VAR,      
                                sizeof(void*), {}, 1, 8);
        cre_Func = new CRE_Type("Func", T_ID_FUNC,   
                                sizeof(void*), {}, 1, 9);
        cre_Literal = new CRE_Type("Literal", T_ID_LITERAL,
                                sizeof(void*), {}, 1, 10);
        cre_Conditions = new CRE_Type("Conditions", T_ID_CONDITIONS,
                                sizeof(void*), {}, 1, 11);
        cre_Rule = new CRE_Type("Rule", T_ID_RULE,
                                sizeof(void*), {}, 1, 12);

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
    return cre_builtins;
}

// void destroy_builtins(){

// }

DefferedType::DefferedType(std::string_view _name) :
    CRE_Type(_name, T_ID_UNDEFINED, 0, {}, 0, 0) {

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

// ------------------------------------------------------------
// : Flags / FlagGroup


inline int get_builtin_flag_id(const std::string_view& name){
    /*  */if(name == "unique_id"){
        return BIFLG_UNIQUE_ID;
    }else if(name == "visible"){
        return BIFLG_VISIBLE_ID;
    }else if(name == "semantic"){
        return BIFLG_SEMANTIC_ID;
    }else if(name == "verbosity"){
        return BIFLG_VERBOSITY;
    }
    return -1;
}


inline uint8_t get_builtin_flag_width(uint8_t flag){
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

void set_builtin_flag(uint64_t* flags, uint64_t flag_n, uint64_t val) {
    uint64_t width = get_builtin_flag_width(flag_n); // should inline const
    uint64_t val_mask = ((1 << width) - 1);
    *flags = (*flags & ~uint64_t(val_mask << flag_n)) |
                        uint64_t((val & val_mask) << flag_n);

    // std::bitset<10> btmp(val_mask);
    // std::bitset<10> tmp(*flags);    
    // std::cout << "SET FLAG(" << tmp  << ", [" << int(flag_n) << "]->" << (val & val_mask) << ", width=" << width << ", " << "mask=" << btmp << ")" << std::endl;
}

uint64_t get_builtin_flag(const uint64_t* flags, uint64_t flag_n) {
    uint64_t width = get_builtin_flag_width(flag_n); // should inline const
    uint64_t val_mask = ((1 << width) - 1) << flag_n;
    uint64_t out = (val_mask & *flags) >> (flag_n);


    // std::bitset<10> btmp(val_mask);    
    // std::bitset<10> tmp(ms->builtin_flags);    
    // std::cout << "GET FLAG(" << tmp << ", " << int(flag_n) << ", width=" << width << ", " << "mask=" << btmp << ")=" << out << std::endl;
    return out;
}

// HashMap<std::string, Item> parse_builtin_flags(
//     uint64_t* builtin_ptr,
//     const HashMap<std::string, Item>& flags,
//     bool as_mask){

//     HashMap<std::string, Item> custom_flags = {};
//     for (auto& [key, value] : flags){
//         int flag_id = get_builtin_flag_id(key);
//         if(flag_id != -1){
//             // All builtin flags are interger like 
//             if(value.t_id != T_ID_BOOL and value.t_id != T_ID_INT){
//                 throw std::runtime_error("Bad value type for builtin flag `" + key + "`.");
//             } 
//             uint64_t val; 
//             if(as_mask){
//                 val = ~0; // all bits=1;
//             }else{
//                 val = item_get_int(value);    
//             }
            
//             set_builtin_flag(builtin_ptr, flag_id, val);    
//         }else{
//             custom_flags[key] = value;    
//         }
//     }    
//     return custom_flags;
// }



void FlagGroup::assign_flag(std::string_view key, Item value){
    int flag_id = get_builtin_flag_id(key);

    // cout << "ASSIGN:" << key << endl; 
    if(flag_id != -1){
        // All builtin flags are interger like 
        if(value.t_id != T_ID_BOOL and value.t_id != T_ID_INT){
            throw std::runtime_error("Bad value type for builtin flag `" + std::string(key) + "`.");
        } 
        uint64_t val = value.as_int();
        set_builtin_flag(&this->builtin_flags,      flag_id, val);
        set_builtin_flag(&this->builtin_flags_mask, flag_id, ~0);    
        // cout << "IS BUILTIN:" << key << endl; 
    }else{
        this->custom_flags[std::string(key)] = value;
    }

    // cout << "GET:" << this->get_flag("unique_id") << endl;
}


void FlagGroup::assign(const HashMap<std::string, Item>& flags){
    // std::cout << "START ASSIGN" << std::endl;
    // HashMap<std::string, Item> custom_flags = {};
    for (auto& [key, value] : flags){
        assign_flag(key, value);
    }
}

Item FlagGroup::get_flag(std::string_view key) const{
    int flag_id = get_builtin_flag_id(key);
    if(flag_id != -1){
        return get_builtin_flag(&builtin_flags, flag_id);
    }else{
        auto itr = this->custom_flags.find(key);
        if(itr != this->custom_flags.end()){
            return itr->second;   
        }else{
            // TODO: Return Undef
            return Item(nullptr);
        }
    }
}


FlagGroup::FlagGroup(const HashMap<std::string, Item>& flags){   
    this->builtin_flags = 0;
    this->builtin_flags_mask = 0;
    this->custom_flags = {};
    this->assign(flags);
}

FlagGroup::FlagGroup(const FlagGroup& flags) :
    builtin_flags(flags.builtin_flags), 
    builtin_flags_mask(flags.builtin_flags),
    custom_flags(flags.custom_flags){
}

// FlagGroup::FlagGroup(std::initializer_list<std::tuple<std::string, Item>> flags){
//     for (auto& [key, value] : flags){
//         assign_flag(key, value);
//     }
// }
    // this->builtin_flags = flags.builtin_flags;
    // this->builtin_flags_mask = flags.builtin_flags_mask;
    // this->custom_flags = {};
    // this->assign(flags);
// }

bool FlagGroup::has_flags(const FlagGroup& other) const{
    uint64_t unmatched = (
        (this->builtin_flags ^
         other.builtin_flags) &
         other.builtin_flags_mask
    );

    // std::cout << "This:  " << std::bitset<10>(this->builtin_flags) << std::endl <<
    //         "Other: " << std::bitset<10>(other.builtin_flags) << std::endl <<
    //         "Mask:  " << std::bitset<10>(other.builtin_flags_mask) << std::endl <<
    //         "Out:   " << std::bitset<10>(unmatched) << std::endl;

    if(unmatched != 0) {return false;}

    for (auto& [key, val] : this->custom_flags){
        auto it = other.custom_flags.find(key);
        if (it == other.custom_flags.end()) {
            return false;
        }
        if((*it).second != val){
            return false;
        }
    }
    return true;
}

} // NAMESPACE_END(cre)
