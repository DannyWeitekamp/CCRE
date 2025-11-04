#include "../include/ref.h"
#include "../include/func.h"

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


bool _And(bool a, bool b){
	return a & b;
}
ref<Func> And = define_func<_And>("And", "And({},{})", "({} & {})");

int64_t _AddInts(int64_t a, int64_t b){
	return a+b;
}
ref<Func> AddInts = define_func<_AddInts>("AddInts", "AddInts({},{})", "({} + {})");

double _Add(double a, double b){
	return a+b;
}
ref<Func> Add = define_func<_Add>("Add", "Add({},{})", "({} + {})");

} // NAMESPACE_END(cre)

