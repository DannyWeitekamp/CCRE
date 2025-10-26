#pragma once
#include <cstdint>

namespace cre {

// Constant declarations
const uint16_t T_ID_UNDEF =       0;
const uint16_t T_ID_NONE =        1;
const uint16_t T_ID_BOOL =        2;
const uint16_t T_ID_INT =         3;
const uint16_t T_ID_FLOAT =       4;
const uint16_t T_ID_PTR =         5;
const uint16_t T_ID_STR =         6;
const uint16_t T_ID_OBJ =         7;
const uint16_t T_ID_FACT =        8;
const uint16_t T_ID_FACTSET =     9;
const uint16_t T_ID_VAR =         10;
const uint16_t T_ID_FUNC =        11;
const uint16_t T_ID_LITERAL =     12;
const uint16_t T_ID_CONDITIONS =  13;
const uint16_t T_ID_RULE =        14;


inline bool t_id_is_primitive(uint16_t t_id){
	return (t_id >= T_ID_BOOL && 
            t_id <= T_ID_STR);
}

inline bool t_id_is_numerical(uint16_t t_id){
	return (t_id >= T_ID_BOOL && 
            t_id <= T_ID_FLOAT);
}


inline bool t_id_is_ptr(uint16_t t_id){
	return (t_id == T_ID_PTR || 
            t_id >= T_ID_OBJ);
}


} // NAMESPACE_END(cre)
