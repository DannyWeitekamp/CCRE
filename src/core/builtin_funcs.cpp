#include "../include/ref.h"
#include "../include/func.h"
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
bool _Equals(double a, double b){
	return a == b;
}
FuncRef Equals = define_func<_Equals>("Equals", "Equals({},{})", "{} == {}", "{} != {}");

bool _EqualsInt(int64_t a, int64_t b){
	return a == b;
}
FuncRef EqualsInt = define_func<_EqualsInt>("EqualsInt", "EqualsInt({},{})", "{} == {}", "{} != {}");

bool _EqualsStr(const StrBlock& a, const StrBlock& b){
	cout << "EqualsStr: " << a.view << " == " << b.view << endl;
	return a.view == b.view;
}
FuncRef EqualsStr = define_func<_EqualsStr>("EqualsStr", "EqualsStr({},{})", "{} == {}", "{} != {}");

// LessThan <
bool _LessThan(double a, double b){
	return a < b;
}
FuncRef LessThan = define_func<_LessThan>("LessThan", "LessThan({},{})", "{} < {}");

bool _LessThanInt(int64_t a, int64_t b){
	return a < b;
}
FuncRef LessThanInt = define_func<_LessThanInt>("LessThanInt", "LessThanInt({},{})", "{} < {}");

bool _LessThanStr(const StrBlock& a, const StrBlock& b){
	return a.view < b.view;
}

FuncRef LessThanStr = define_func<_LessThanStr>("LessThanStr", "LessThanStr({},{})", "{} < {}");

// GreaterThan >
bool _GreaterThan(double a, double b){
	return a > b;
}
FuncRef GreaterThan = define_func<_GreaterThan>("GreaterThan", "GreaterThan({},{})", "{} > {}");

bool _GreaterThanInt(int64_t a, int64_t b){
	return a > b;
}
FuncRef GreaterThanInt = define_func<_GreaterThanInt>("GreaterThanInt", "GreaterThanInt({},{})", "{} > {}");

bool _GreaterThanStr(const StrBlock& a, const StrBlock& b){
	return a.view > b.view;
}
FuncRef GreaterThanStr = define_func<_GreaterThanStr>("GreaterThanStr", "GreaterThanStr({},{})", "{} > {}");

// LessThanOrEqual <=
bool _LessThanOrEqual(double a, double b){
	return a <= b;
}
FuncRef LessThanOrEqual = define_func<_LessThanOrEqual>("LessThanOrEqual", "LessThanOrEqual({},{})", "{} <= {}");

bool _LessThanOrEqualInt(int64_t a, int64_t b){
	return a <= b;
}
FuncRef LessThanOrEqualInt = define_func<_LessThanOrEqualInt>("LessThanOrEqualInt", "LessThanOrEqualInt({},{})", "{} <= {}");

bool _LessThanOrEqualStr(const StrBlock& a, const StrBlock& b){
	return a.view <= b.view;
}
FuncRef LessThanOrEqualStr = define_func<_LessThanOrEqualStr>("LessThanOrEqualStr", "LessThanOrEqualStr({},{})", "{} <= {}");

// GreaterThanOrEqual >=
bool _GreaterThanOrEqual(double a, double b){
	return a >= b;
}
FuncRef GreaterThanOrEqual = define_func<_GreaterThanOrEqual>("GreaterThanOrEqual", "GreaterThanOrEqual({},{})", "{} >= {}");

bool _GreaterThanOrEqualInt(int64_t a, int64_t b){
	return a >= b;
}
FuncRef GreaterThanOrEqualInt = define_func<_GreaterThanOrEqualInt>("GreaterThanOrEqualInt", "GreaterThanOrEqualInt({},{})", "{} >= {}");

bool _GreaterThanOrEqualStr(const StrBlock& a, const StrBlock& b){
	return a.view >= b.view;
}
FuncRef GreaterThanOrEqualStr = define_func<_GreaterThanOrEqualStr>("GreaterThanOrEqualStr", "GreaterThanOrEqualStr({},{})", "{} >= {}");


// Logical operators
// And &
bool _And(bool a, bool b){
	return a & b;
}
FuncRef And = define_func<_And>("And", "And({},{})", "{} & {}");

// Or |
bool _Or(bool a, bool b){
	return a | b;
}
FuncRef Or = define_func<_Or>("Or", "Or({},{})", "{} | {}");

// Xor ^
bool _Xor(bool a, bool b){
	return a ^ b;
}
FuncRef Xor = define_func<_Xor>("Xor", "Xor({},{})", "{} ^ {}");

// Not ~
bool _Not(bool a){
	return !a;
}
FuncRef NotFunc = define_func<_Not>("Not", "Not({})", "~{}");

// Add +
int64_t _AddInts(int64_t a, int64_t b){
	return a+b;
}
FuncRef AddInts = define_func<_AddInts>("AddInts", "AddInts({},{})", "{} + {}");

double _Add(double a, double b){
	return a+b;
}
FuncRef Add = define_func<_Add>("Add", "Add({},{})", "{} + {}");

// Subtract -
int64_t _SubtractInts(int64_t a, int64_t b){
	return a-b;
}
FuncRef SubtractInts = define_func<_SubtractInts>("SubtractInts", "SubtractInts({},{})", "{} - {}");

double _Subtract(double a, double b){
	return a-b;
}
FuncRef Subtract = define_func<_Subtract>("Subtract", "Subtract({},{})", "{} - {}");

// Multiply *
int64_t _MultiplyInts(int64_t a, int64_t b){
	return a*b;
}
FuncRef MultiplyInts = define_func<_MultiplyInts>("MultiplyInts", "MultiplyInts({},{})", "{} * {}");

double _Multiply(double a, double b){
	return a*b;
}
FuncRef Multiply = define_func<_Multiply>("Multiply", "Multiply({},{})", "{} * {}");

// Divide /
int64_t _DivideInts(int64_t a, int64_t b){
	return a/b;
}
FuncRef DivideInts = define_func<_DivideInts>("DivideInts", "DivideInts({},{})", "{} / {}");

double _Divide(double a, double b){
	return a/b;
}
FuncRef Divide = define_func<_Divide>("Divide", "Divide({},{})", "{} / {}");

double _FloorDivide(double a, double b){
	return std::floor(a/b);
}
FuncRef FloorDivide = define_func<_FloorDivide>("FloorDivide", "FloorDivide({},{})", "{} // {}");

// Modulo %
int64_t _ModuloInts(int64_t a, int64_t b){
	return a%b;
}
FuncRef ModuloInts = define_func<_ModuloInts>("ModuloInts", "ModuloInts({},{})", "{} % {}");

double _Modulo(double a, double b){
	return fmod(a, b);
}
FuncRef Modulo = define_func<_Modulo>("Modulo", "Modulo({},{})", "{} % {}");

// Power/Exponentiation **
double _Pow(double a, double b){
	return std::pow(a, b);
}
FuncRef Pow = define_func<_Pow>("Pow", "Pow({},{})", "{} ** {}");

// Unary Negation -x
int64_t _NegateInt(int64_t a){
	return -a;
}
FuncRef NegateInt = define_func<_NegateInt>("NegateInt", "NegateInt({})", "-{}");

double _Negate(double a){
	return -a;
}
FuncRef Negate = define_func<_Negate>("Negate", "Negate({})", "-{}");

// String operations
std::string _Concat(const StrBlock& _a, const StrBlock& _b){
	std::string_view a = _a.view;
	std::string_view b = _b.view;
	std::stringstream ss;
	ss << a << b;
	return ss.str();
}
FuncRef Concat = define_func<_Concat>("Concat", "Concat({},{})", "{} + {}");

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

