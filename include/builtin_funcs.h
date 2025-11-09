#pragma once

#include "../include/ref.h"
#include "../include/func.h"


namespace cre {

// ref<Func> bool_to_str;
// ref<Func> int_to_str;
// ref<Func> float_to_str;
// ref<Func> str_to_bool;
// ref<Func> str_to_int;
// ref<Func> str_to_float;
// ref<Func> bool_to_int;
// ref<Func> bool_to_float;
// ref<Func> int_to_bool;
// ref<Func> int_to_float;
// ref<Func> float_to_bool;
// ref<Func> float_to_int;


extern FuncRef Equals;
extern FuncRef EqualsInt;
extern FuncRef EqualsStr;
extern FuncRef LessThan;
extern FuncRef GreaterThan;
extern FuncRef LessThanOrEqual;
extern FuncRef GreaterThanOrEqual;
extern FuncRef And;
extern FuncRef Or;
extern FuncRef Xor;
extern FuncRef Not;
extern FuncRef AddInts;
extern FuncRef Add;
extern FuncRef SubtractInts;
extern FuncRef Subtract;
extern FuncRef MultiplyInts;
extern FuncRef Multiply;
extern FuncRef DivideInts;
extern FuncRef Divide;
extern FuncRef FloorDivide;
extern FuncRef Modulo;
extern FuncRef ModuloInts;
extern FuncRef Pow;
extern FuncRef NegateInt;
extern FuncRef Negate;
extern FuncRef Concat;
extern FuncRef Slice;

} // NAMESPACE_END(cre)
