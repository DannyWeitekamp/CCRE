#ifndef _CRE_TYPES_H_
#define _CRE_TYPES_H_

#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <sstream>

// Forward declarations
class CRE_Context;
struct Type;

// Type declaration
struct Type {
    std::string name;
    uint16_t t_id;
    uint8_t builtin;
    std::vector<Type*> sub_types;
    std::vector<std::tuple<std::string, Type*>> members;

    Type(std::string _name, 
         std::vector<std::tuple<std::string, Type*>> _members = {},
         std::vector<Type*> _sub_types = {}, 
         uint8_t _builtin = 0);
};

// Function declarations
std::ostream& operator<<(std::ostream& outs, const Type& t);
std::string to_string(const Type& value);

// CRE_Context declaration
struct CRE_Context {
    std::string name;
    std::vector<Type> types;
    std::unordered_map<std::string, uint16_t> type_name_map;

    CRE_Context(std::string _name);
    uint16_t _add_type(Type t);
};

Type* define_type(CRE_Context* context, 
                  std::string name, 
                  std::vector<std::tuple<std::string, Type*>> members = {},
                  std::vector<Type*> sub_types = {}, 
                  uint8_t builtin = 0);

// Constant declarations
const uint16_t T_ID_UNDEFINED = 1;
const uint16_t T_ID_EMPTY_BLOCK = 2;
const uint16_t T_ID_BOOL = 3;
const uint16_t T_ID_INT = 4;
const uint16_t T_ID_FLOAT = 5;
const uint16_t T_ID_STR = 6;
const uint16_t T_ID_OBJ = 7;
const uint16_t T_ID_STATE = 8;
const uint16_t T_ID_FACT = 9;
const uint16_t T_ID_VAR = 10;
const uint16_t T_ID_FUNC = 11;
const uint16_t T_ID_LITERAL = 12;
const uint16_t T_ID_CONDITIONS = 13;
const uint16_t T_ID_RULE = 14;

// Global variable declarations
extern Type* cre_undefined;
extern Type* cre_bool;
extern Type* cre_int;
extern Type* cre_float;
extern Type* cre_str;
extern Type* cre_obj;
extern Type* cre_Fact;
extern Type* cre_Var;
extern Type* cre_Func;
extern Type* cre_Literal;
extern Type* cre_Conditions;
extern Type* cre_Rule;

extern std::vector<Type*> cre_builtins;
extern CRE_Context default_context;

#endif /* _CRE_TYPES_H_ */
