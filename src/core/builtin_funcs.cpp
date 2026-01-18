#include "../include/ref.h"
#include "../include/func.h"
#include "../include/builtin_funcs.h"
#include <cmath>
#include <sstream>

namespace cre {
// std::string _bool_to_str(bool x){
// 	return std::to_string(x);
// }
// bool_to_str = define_func<_bool_to_str>("str");

// std::string _int_to_str(int64_t x){
// 	return std::to_string(x);
// }
// int_to_str = define_func<_int_to_str>("str");

// std::string _float_to_str(double x){
// 	return std::to_string(x);
// }
// float_to_str = define_func<_float_to_str>("str");


// bool _str_to_bool(std::string_view x){
// 	return ??
// }
// str_to_bool = define_func<_str_to_bool>("bool");

// int64_t _str_to_int(std::string_view x){
// 	return ??
// }
// str_to_int = define_func<_str_to_int>("int");

// double _str_to_float(std::string_view x){
// 	return ??
// }
// str_to_float = define_func<_str_to_float>("float");


// int64_t _bool_to_int(bool x){
// 	return int(x);
// }
// bool_to_int = define_func<_bool_to_int>("int");


// double _bool_to_float(bool x){
// 	return float(x);
// }
// bool_to_float = define_func<_bool_to_float>("float");

// bool _int_to_bool(int64_t x){
// 	return bool(x);
// }
// int_to_bool = define_func<_int_to_bool>("bool");

// double _int_to_float(int64_t x){
// 	return float(x);
// }
// int_to_bool = define_func<_int_to_bool>("float");

// bool _float_to_bool(double x){
// 	return bool(x);
// }
// float_to_bool = define_func<_float_to_bool>("bool");

// int64_t _float_to_int(double x){
// 	return int64_t(x);
// }
// float_to_int = define_func<_float_to_int>("int");

// Equals ==
bool _EqualsFloat(double a, double b){
	// cout << "Equals: " << a << " == " << b << endl;
	return a == b;
}
FuncRef EqualsFloat = define_func<_EqualsFloat>("EqualsFloat", "EqualsFloat({},{})", "{} == {}", "{} != {}");

bool _EqualsInt(int64_t a, int64_t b){	
	return a == b;
}
FuncRef EqualsInt = define_func<_EqualsInt>("EqualsInt", "EqualsInt({},{})", "{} == {}", "{} != {}");


bool _EqualsStr(const StrBlock& a, const StrBlock& b){
	// cout << "EqualsStr: " << a.view << " == " << b.view << endl;
	return a.view == b.view;
}
FuncRef EqualsStr = define_func<_EqualsStr>("EqualsStr", "EqualsStr({},{})", "{} == {}", "{} != {}");

FuncRef Equals(Item& a, Item& b){
	return resolve_func_impl("Equals", a, b, EqualsFloat, EqualsInt, EqualsStr);
}

// LessThan <
bool _LessThanFloat(double a, double b){
	return a < b;
}
FuncRef LessThanFloat = define_func<_LessThanFloat>("LessThanFloat", "LessThanFloat({},{})", "{} < {}");

bool _LessThanInt(int64_t a, int64_t b){
	return a < b;
}
FuncRef LessThanInt = define_func<_LessThanInt>("LessThanInt", "LessThanInt({},{})", "{} < {}");

bool _LessThanStr(const StrBlock& a, const StrBlock& b){
	return a.view < b.view;
}

FuncRef LessThanStr = define_func<_LessThanStr>("LessThanStr", "LessThanStr({},{})", "{} < {}");

// --  GreaterThan > -- //
bool _GreaterThanFloat(double a, double b){
	return a > b;
}
FuncRef GreaterThanFloat = define_func<_GreaterThanFloat>("GreaterThanFloat", "GreaterThanFloat({},{})", "{} > {}");

bool _GreaterThanInt(int64_t a, int64_t b){
	return a > b;
}
FuncRef GreaterThanInt = define_func<_GreaterThanInt>("GreaterThanInt", "GreaterThanInt({},{})", "{} > {}");

bool _GreaterThanStr(const StrBlock& a, const StrBlock& b){
	return a.view > b.view;
}
FuncRef GreaterThanStr = define_func<_GreaterThanStr>("GreaterThanStr", "GreaterThanStr({},{})", "{} > {}");



// -- LessThanOrEqual <= -- //
bool _LessThanOrEqualFloat(double a, double b){
	return a <= b;
}
FuncRef LessThanOrEqualFloat = define_func<_LessThanOrEqualFloat>("LessThanOrEqualFloat", "LessThanOrEqualFloat({},{})", "{} <= {}");

bool _LessThanOrEqualInt(int64_t a, int64_t b){
	return a <= b;
}
FuncRef LessThanOrEqualInt = define_func<_LessThanOrEqualInt>("LessThanOrEqualInt", "LessThanOrEqualInt({},{})", "{} <= {}");

bool _LessThanOrEqualStr(const StrBlock& a, const StrBlock& b){
	return a.view <= b.view;
}
FuncRef LessThanOrEqualStr = define_func<_LessThanOrEqualStr>("LessThanOrEqualStr", "LessThanOrEqualStr({},{})", "{} <= {}");

// -- GreaterThanOrEqual >= -- //
bool _GreaterThanOrEqualFloat(double a, double b){
	return a >= b;
}
FuncRef GreaterThanOrEqualFloat = define_func<_GreaterThanOrEqualFloat>("GreaterThanOrEqualFloat", "GreaterThanOrEqualFloat({},{})", "{} >= {}");

bool _GreaterThanOrEqualInt(int64_t a, int64_t b){
	return a >= b;
}
FuncRef GreaterThanOrEqualInt = define_func<_GreaterThanOrEqualInt>("GreaterThanOrEqualInt", "GreaterThanOrEqualInt({},{})", "{} >= {}");

bool _GreaterThanOrEqualStr(const StrBlock& a, const StrBlock& b){
	return a.view >= b.view;
}
FuncRef GreaterThanOrEqualStr = define_func<_GreaterThanOrEqualStr>("GreaterThanOrEqualStr", "GreaterThanOrEqualStr({},{})", "{} >= {}");


// Logical operators
// -- And & -- //
bool _And(bool a, bool b){
	return a & b;
}
FuncRef And = define_func<_And>("And", "And({},{})", "{} & {}");

// -- Or | -- //
bool _Or(bool a, bool b){
	return a | b;
}
FuncRef Or = define_func<_Or>("Or", "Or({},{})", "{} | {}");

// -- Xor ^ -- //
bool _Xor(bool a, bool b){
	return a ^ b;
}
FuncRef Xor = define_func<_Xor>("Xor", "Xor({},{})", "{} ^ {}");

// -- Not ~ -- //
bool _Not(bool a){
	return !a;
}
FuncRef NotFunc = define_func<_Not>("Not", "Not({})", "~{}");

// -- Add + -- //
int64_t _AddInt(int64_t a, int64_t b){
	return a+b;
}
FuncRef AddInt = define_func<_AddInt>("AddInt", "AddInt({},{})", "{} + {}");

double _AddFloat(double a, double b){
	return a+b;
}
FuncRef AddFloat = define_func<_AddFloat>("AddFloat", "AddFloat({},{})", "{} + {}");

// -- Subtract - -- //
int64_t _SubtractInt(int64_t a, int64_t b){
	return a-b;
}
FuncRef SubtractInt = define_func<_SubtractInt>("SubtractInt", "SubtractInt({},{})", "{} - {}");

double _SubtractFloat(double a, double b){
	return a-b;
}
FuncRef SubtractFloat = define_func<_SubtractFloat>("SubtractFloat", "SubtractFloat({},{})", "{} - {}");

// -- Multiply * -- //
int64_t _MultiplyInt(int64_t a, int64_t b){
	return a*b;
}
FuncRef MultiplyInt = define_func<_MultiplyInt>("MultiplyInt", "MultiplyInt({},{})", "{} * {}");

double _MultiplyFloat(double a, double b){
	return a*b;
}
FuncRef MultiplyFloat = define_func<_MultiplyFloat>("MultiplyFloat", "MultiplyFloat({},{})", "{} * {}");

// -- Divide / -- //
int64_t _DivideInt(int64_t a, int64_t b){
	return a/b;
}
FuncRef DivideInt = define_func<_DivideInt>("DivideInt", "DivideInt({},{})", "{} / {}");

double _DivideFloat(double a, double b){
	return a/b;
}
FuncRef DivideFloat = define_func<_DivideFloat>("DivideFloat", "DivideFloat({},{})", "{} / {}");

double _FloorDivideFloat(double a, double b){
	return std::floor(a/b);
}
FuncRef FloorDivideFloat = define_func<_FloorDivideFloat>("FloorDivideFloat", "FloorDivideFloat({},{})", "{} // {}");

// -- Modulo % -- //
int64_t _ModuloInt(int64_t a, int64_t b){
	return a%b;
}
FuncRef ModuloInt = define_func<_ModuloInt>("ModuloInt", "ModuloInt({},{})", "{} % {}");

double _ModuloFloat(double a, double b){
	return fmod(a, b);
}
FuncRef ModuloFloat = define_func<_ModuloFloat>("ModuloFloat", "ModuloFloat({},{})", "{} % {}");

// -- Power/Exponentiation ** -- //
int64_t _PowInt(int64_t a, int64_t b){
	return std::pow(a, b);
}
FuncRef PowInt = define_func<_PowInt>("PowInt", "PowInt({},{})", "{} ** {}");

double _PowFloat(double a, double b){
	return std::pow(a, b);
}
FuncRef PowFloat = define_func<_PowFloat>("PowFloat", "PowFloat({},{})", "{} ** {}");

// -- Unary Negation -x -- //
int64_t _NegateInt(int64_t a){
	return -a;
}
FuncRef NegateInt = define_func<_NegateInt>("NegateInt", "NegateInt({})", "-{}");

double _NegateFloat(double a){
	return -a;
}
FuncRef NegateFloat = define_func<_NegateFloat>("NegateFloat", "NegateFloat({})", "-{}");

// String operations
// -- Concat + -- //
std::string _Concat(const StrBlock& _a, const StrBlock& _b){
	std::string_view a = _a.view;
	std::string_view b = _b.view;
	std::stringstream ss;
	ss << a << b;
	return ss.str();
}
FuncRef Concat = define_func<_Concat>("Concat", "Concat({},{})", "{} + {}");

// -- Slice [:] -- //
std::string _Slice(const StrBlock& _str, int64_t start, int64_t end){
	std::string_view str = _str.view;
	size_t len = str.length();
	
	// Handle negative indices
	size_t s = (start < 0) ? (int64_t(len + start) >= 0 ? (len + start) : 0) : static_cast<size_t>(start);
	size_t e = (end < 0) ? (int64_t(len + end) >= 0 ? (len + end) : 0) : static_cast<size_t>(end);
	
	// Clamp to bounds
	if (s > len) s = len;
	if (e > len) e = len;
	if (s > e) s = e;
	
	return std::string(str.substr(s, e - s));
}
FuncRef Slice = define_func<_Slice>("Slice", "Slice({},{},{})", "{}[{}:{}]");

} // NAMESPACE_END(cre)

