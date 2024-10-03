#include "../include/types.h"

using namespace std;

// Type constructor implementation
Type::Type(string _name, 
           vector<tuple<string, Type*>> _members,
           vector<Type*> _sub_types, 
           uint8_t _builtin)
    : name(_name), members(_members), sub_types(_sub_types), builtin(_builtin) {}

// Operator<< implementation
ostream& operator<<(ostream& outs, const Type& t) {
    return outs << t.name;
}

// to_string implementation
string to_string(const Type& value) {
    ostringstream ss;
    ss << value;
    return ss.str();
}

// CRE_Context constructor implementation
CRE_Context::CRE_Context(string _name) : name(_name) {
    for (auto t : cre_builtins) {
        _add_type(*t);
    }
}

// CRE_Context::_add_type implementation
uint16_t CRE_Context::_add_type(Type t) {
    uint16_t t_id;
    if (type_name_map.find(t.name) == type_name_map.end()) {
        t_id = types.size();
        types.push_back(t);
    } else {
        t_id = type_name_map[t.name];
        types[t_id] = t;
    }    
    t.t_id = t_id;
    return t_id;
}

// define_type implementation
Type* define_type(CRE_Context* context, 
                  string name, 
                  vector<tuple<string, Type*>> members,
                  vector<Type*> sub_types, 
                  uint8_t builtin) {
    Type t(name, members, sub_types, builtin);
    uint16_t t_id = context->_add_type(t);
    return &context->types[t_id];
}



// Global variable definitions
Type* cre_undefined = new Type("undefined");
Type* cre_bool = new Type("bool");
Type* cre_int = new Type("int");
Type* cre_float = new Type("float");
Type* cre_str = new Type("str");
Type* cre_obj = new Type("obj");
Type* cre_Fact = new Type("Fact");
Type* cre_Var = new Type("Var");
Type* cre_Func = new Type("Func");
Type* cre_Literal = new Type("Literal");
Type* cre_Conditions = new Type("Conditions");
Type* cre_Rule = new Type("Rule");

vector<Type*> cre_builtins = {
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

CRE_Context default_context("default");
