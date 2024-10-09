#include "../include/types.h"
#include "../include/context.h"
#include <iostream>

// using namespace std;

// Type constructor implementation
CRE_Type::CRE_Type(std::string_view _name, 
           vector<CRE_Type*> _sub_types, 
           uint8_t _builtin)
    : name(std::string(_name)), sub_types(_sub_types), builtin(_builtin) {}


FactType::FactType(std::string_view _name, 
           vector<CRE_Type*> _sub_types, 
           vector<tuple<std::string, CRE_Type*>> _members)
    : CRE_Type(_name, _sub_types, 0), members(_members) {}




// Operator<< implementation
ostream& operator<<(ostream& outs, const FactType& t) {
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
                  std::vector<CRE_Type*> sub_types,
                  CRE_Context* context) {
    if(context == nullptr){
        context = default_context;
    };
    CRE_Type* t = new CRE_Type(name, sub_types, 1);
    // cout << "DEFINE TYPE" << t->name << endl;
    uint16_t t_id = context->_add_type(t);
    return context->types[t_id];
}

FactType* define_fact(std::string_view name, 
                  std::vector<std::tuple<std::string, CRE_Type*>> members,
                  std::vector<CRE_Type*> sub_types,
                  CRE_Context* context) {
    if(context == nullptr){
        context = default_context;
    };
    // cout << "DEFINE FACT0: " << name << endl;
    FactType* t = new FactType(name, sub_types, members);
    // cout << "DEFINE FACT" << t->name << endl;
    uint16_t t_id = context->_add_type(t);
    return (FactType*) context->types[t_id];
}

size_t FactType::get_index(std::string_view key){
    for(size_t i = 0; i < this->members.size(); i++){
        if(std::get<0>(this->members[i]) == key){
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
CRE_Type* cre_Var;// = new CRE_Type("Var",{}, 1);
CRE_Type* cre_Func;// = new CRE_Type("Func",{}, 1);
CRE_Type* cre_Literal;// = new CRE_Type("Literal",{}, 1);
CRE_Type* cre_Conditions;// = new CRE_Type("Conditions",{}, 1);
CRE_Type* cre_Rule;// = new CRE_Type("Rule",{}, 1);


std::vector<CRE_Type*> cre_builtins;// = {};

void ensure_builtins(){
    if(cre_builtins.size() == 0){
        cre_undefined = new CRE_Type("undefined",{}, 1);
        cre_bool = new CRE_Type("bool",{}, 1);
        cre_int = new CRE_Type("int",{}, 1);
        cre_float = new CRE_Type("float",{}, 1);
        cre_str = new CRE_Type("str",{}, 1);
        cre_obj = new CRE_Type("obj",{}, 1);
        cre_Fact = new CRE_Type("Fact",{}, 1);
        cre_Var = new CRE_Type("Var",{}, 1);
        cre_Func = new CRE_Type("Func",{}, 1);
        cre_Literal = new CRE_Type("Literal",{}, 1);
        cre_Conditions = new CRE_Type("Conditions",{}, 1);
        cre_Rule = new CRE_Type("Rule",{}, 1);

        // cout << "INIT builtins";
        cre_builtins = {
            cre_undefined,
            cre_bool,
            cre_int,
            cre_float,
            cre_str,
            cre_obj,
            cre_Fact,
            cre_Var,
            cre_Func,
            cre_Literal,
            cre_Conditions,
            cre_Rule,
        };    
    }
}

