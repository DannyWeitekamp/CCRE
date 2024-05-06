#ifndef _CRE_TYPES_H_
#define _CRE_TYPES_H_

#include <iostream>
#include <string>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <sstream>
using namespace std;

// forward declaration
class CRE_Context;

struct Type{
  // The name of the type
  string name;

  uint16_t t_id; 
  uint8_t builtin; 

  // Subtypes of the type
  vector<Type*> sub_types;

  // Attribute Member Types 
  vector<tuple<string, Type*>> members;

  // Constructor
  Type(string                        _name, 
       vector<tuple<string, Type*>>  _members={},
       vector<Type*>                 _sub_types={}, 
       uint8_t                       _builtin=0
       ){
    name = _name;
    members = _members;
    sub_types = _sub_types;
    builtin = _builtin;
  };
};

// Streams and to_string()
ostream& operator << ( ostream& outs, const Type& t){
  return outs << t.name;
}

string to_string( const Type& value ){
  ostringstream ss;
  ss << value;
  return ss.str();
}

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

// enum class BUILTIN_T_IDS : uint16_t {
//   T_ID_UNDEFINED,
//   T_ID_EMPTY_BLOCK,
//   T_ID_BOOL,
//   T_ID_INT,
//   T_ID_FLOAT,
//   T_ID_STR,
//   T_ID_OBJ,
//   T_ID_STATE,
//   T_ID_FACT,
//   T_ID_VAR,
//   T_ID_FUNC,
//   T_ID_LITERAL,
//   T_ID_CONDITIONS,
//   T_ID_RULE,
// };
// using enum BUILTIN_T_IDS;
// {
  
// };
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

struct CRE_Context{
  string name;
  vector<Type> types;
  unordered_map<string, uint16_t> type_name_map;
  

  // Constructor
  CRE_Context(string _name){
    name = _name;
    types = {};
    type_name_map = {};

    for (auto t : cre_builtins) {
      _add_type(*t);
    }

  };

  uint16_t _add_type(Type t){
    uint16_t t_id;
    if(type_name_map.find(t.name) == type_name_map.end()){
      t_id = types.size();
      types.push_back(t);
    }else{
      t_id = type_name_map[t.name];
      types[t_id] = t;
    }    
    t.t_id = t_id;
    return t_id;
  };

};

Type* define_type(CRE_Context* context, 
                 string                        name, 
                 vector<tuple<string, Type*>>  members={},
                 vector<Type*>                 sub_types={}, 
                 uint8_t                       builtin=0){

  Type t = Type(name, members, sub_types);
  uint16_t t_id = context->_add_type(t);
  return &context->types[t_id];
}

CRE_Context default_context = CRE_Context("default");


#endif /* _CRE_TYPES_H_ */
