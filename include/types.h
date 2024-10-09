#ifndef _CRE_TYPES_H_
#define _CRE_TYPES_H_

#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <sstream>

// Forward declarations
struct CRE_Context;
struct Type;

// // Alias CRE_dtor_function as pointer to void(void*)
// typedef void (*CRE_dtor_function)(CRE_Obj* ptr);


struct CRE_Type {
    std::string name;
    uint16_t t_id;
    uint8_t builtin;
    std::vector<CRE_Type*> sub_types;

    CRE_Type(std::string_view _name, 
         std::vector<CRE_Type*> _sub_types = {}, 
         uint8_t _builtin = 0);

    
};

// Type declaration
struct FactType : public CRE_Type{
    std::vector<std::tuple<std::string, CRE_Type*>> members;
    size_t get_index(std::string_view key);

    FactType(std::string_view _name, 
         std::vector<CRE_Type*> _sub_types = {}, 
         std::vector<std::tuple<std::string, CRE_Type*>> _members = {});
};

// Function declarations
std::ostream& operator<<(std::ostream& outs, const FactType& t);
std::string to_string(const FactType& value);


CRE_Type* define_type(std::string_view name, 
                  std::vector<CRE_Type*> sub_type={},
                  CRE_Context* context = nullptr);

FactType* define_fact(std::string_view name, 
                  std::vector<std::tuple<std::string, CRE_Type*>> members,
                  std::vector<CRE_Type*> sub_types={},
                  CRE_Context* context=nullptr 
                  ) ;

extern "C" size_t FactType_get_member_index(FactType* type, char* key);

// Constant declarations
const uint16_t T_ID_UNDEFINED = 1;
const uint16_t T_ID_EMPTY_BLOCK = 2;
const uint16_t T_ID_BOOL = 3;
const uint16_t T_ID_INT = 4;
const uint16_t T_ID_FLOAT = 5;
const uint16_t T_ID_STR = 6;
const uint16_t T_ID_OBJ = 7;
const uint16_t T_ID_FACT = 8;
const uint16_t T_ID_FACTSET = 9;
const uint16_t T_ID_VAR = 10;
const uint16_t T_ID_FUNC = 11;
const uint16_t T_ID_LITERAL = 12;
const uint16_t T_ID_CONDITIONS = 13;
const uint16_t T_ID_RULE = 14;

// Global variable declarations
extern CRE_Type* cre_undefined;
extern CRE_Type* cre_bool;
extern CRE_Type* cre_int;
extern CRE_Type* cre_float;
extern CRE_Type* cre_str;
extern CRE_Type* cre_obj;
extern CRE_Type* cre_Fact;
extern CRE_Type* cre_Var;
extern CRE_Type* cre_Func;
extern CRE_Type* cre_Literal;
extern CRE_Type* cre_Conditions;
extern CRE_Type* cre_Rule;

extern std::vector<CRE_Type*> cre_builtins;

void ensure_builtins();
// extern CRE_Context default_context;

#endif /* _CRE_TYPES_H_ */
