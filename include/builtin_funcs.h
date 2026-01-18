#pragma once

#include "../include/ref.h"
#include "../include/func.h"
#include "types.h"


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

template<typename T>
uint16_t t_id_of(T&& x){
    uint16_t t_id = T_ID_UNDEF;
    using DecayT = std::remove_pointer_t<remove_ref_t<std::remove_cvref_t<T>>>;
    if constexpr(is_var_or_func<DecayT>::value){
        t_id = x->eval_t_id();
    }else if constexpr(std::is_base_of_v<Item, DecayT>){
        t_id = x.get_t_id();
        if(t_id == T_ID_VAR) t_id = Item(x).as<Var*>()->eval_t_id();
        if(t_id == T_ID_FUNC) t_id = Item(x).as<Func*>()->eval_t_id();
    }else{
        t_id = to_cre_type<DecayT>()->get_t_id();
    }
    return t_id;
}


template<typename Ta, typename Tb, typename... Fs>
FuncRef resolve_func_impl(std::string_view name, Ta&& a, Tb&& b, Fs&&... funcs){
    
    // Figure out the t_ids of the arguments.
    uint16_t t_id_a = t_id_of(a);
    uint16_t t_id_b = t_id_of(b);
    uint16_t max_t_id = std::max(t_id_a, t_id_b);

    // First pass... try to find an exact match.
    FuncRef result(nullptr);
	([&] {
		FuncRef func = funcs;
		if(max_t_id == func->root_arg_infos[0].type->get_t_id()){
			result = func; return;
		}
	} (), ...);

    if(result.get() != nullptr) return result;

    // Second pass... see if we can cast max_t_id to each impl.
    ([&] {
		FuncRef func = funcs;
		if(cast_allowed(max_t_id, func->root_arg_infos[0].type->get_t_id())){
			result = func; return;
		}
	} (), ...);

    if(result.get() != nullptr) return result;

	// throw std::runtime_error(fmt::format("No implementation found for {}({},{})", name, a, b));
    std::stringstream ss;
    ss << "No implementation found for " << name << "(" << a << ", " << b << ")";
    ss << " with evaluated types " <<  get_cre_type(t_id_a)->to_string() << " and " << get_cre_type(t_id_b)->to_string();
	throw std::runtime_error(ss.str());
    return result; // This should never happen.
}


extern FuncRef EqualsFloat;
extern FuncRef EqualsInt;
extern FuncRef EqualsStr;

// // Specialize for Item arguments (probably saves some compile time)
// inline auto Equals(Item& a, Item& b){
//     return resolve_func_impl("Equals", a, b, EqualsFloat, EqualsInt, EqualsStr)(a,b);
// }
// Generic case
template<typename Ta, typename Tb>
auto Equals(Ta&& a, Tb&& b){
    return resolve_func_impl("Equals", std::forward<Ta>(a), std::forward<Tb>(b),
                             EqualsFloat, EqualsInt, EqualsStr)(a,b);
}
extern FuncRef LessThanFloat;
extern FuncRef LessThanInt;
extern FuncRef LessThanStr;

template<typename Ta, typename Tb>
auto LessThan(Ta&& a, Tb&& b){
    return resolve_func_impl("LessThan", std::forward<Ta>(a), std::forward<Tb>(b),
                             LessThanFloat, LessThanInt, LessThanStr)(a,b);
}
extern FuncRef GreaterThanFloat;
extern FuncRef GreaterThanInt;
extern FuncRef GreaterThanStr;

template<typename Ta, typename Tb>
auto GreaterThan(Ta&& a, Tb&& b){
    return resolve_func_impl("GreaterThan", std::forward<Ta>(a), std::forward<Tb>(b),
                             GreaterThanFloat, GreaterThanInt, GreaterThanStr)(a,b);
}

extern FuncRef LessThanOrEqualFloat;
extern FuncRef LessThanOrEqualInt;
extern FuncRef LessThanOrEqualStr;

template<typename Ta, typename Tb>
auto LessThanOrEqual(Ta&& a, Tb&& b){
    return resolve_func_impl("LessThanOrEqual", std::forward<Ta>(a), std::forward<Tb>(b),
                             LessThanOrEqualFloat, LessThanOrEqualInt, LessThanOrEqualStr)(a,b);
}

extern FuncRef GreaterThanOrEqualFloat;
extern FuncRef GreaterThanOrEqualInt;
extern FuncRef GreaterThanOrEqualStr;

template<typename Ta, typename Tb>
auto GreaterThanOrEqual(Ta&& a, Tb&& b){
    return resolve_func_impl("GreaterThanOrEqual", std::forward<Ta>(a), std::forward<Tb>(b),
                             GreaterThanOrEqualFloat, GreaterThanOrEqualInt, GreaterThanOrEqualStr)(a,b);
}

extern FuncRef And;
extern FuncRef Or;
extern FuncRef Xor;
extern FuncRef NotFunc;
extern FuncRef AddInt;
extern FuncRef AddFloat;

template<typename Ta, typename Tb>
auto Add(Ta&& a, Tb&& b){
    return resolve_func_impl("Add", std::forward<Ta>(a), std::forward<Tb>(b),
                             AddFloat, AddInt)(a,b);
}
extern FuncRef SubtractInt;
extern FuncRef SubtractFloat;

template<typename Ta, typename Tb>
auto Subtract(Ta&& a, Tb&& b){
    return resolve_func_impl("Subtract", std::forward<Ta>(a), std::forward<Tb>(b),
                             SubtractFloat, SubtractInt)(a,b);
}
extern FuncRef MultiplyInt;
extern FuncRef MultiplyFloat;

template<typename Ta, typename Tb>
auto Multiply(Ta&& a, Tb&& b){
    return resolve_func_impl("Multiply", std::forward<Ta>(a), std::forward<Tb>(b),
                             MultiplyFloat, MultiplyInt)(a,b);
}
extern FuncRef DivideInt;
extern FuncRef DivideFloat;

template<typename Ta, typename Tb>
auto Divide(Ta&& a, Tb&& b){
    return resolve_func_impl("Divide", std::forward<Ta>(a), std::forward<Tb>(b),
                             DivideFloat, DivideInt)(a,b);
}
extern FuncRef FloorDivideFloat;
template<typename Ta, typename Tb>
auto FloorDivide(Ta&& a, Tb&& b){
    return resolve_func_impl("FloorDivide", std::forward<Ta>(a), std::forward<Tb>(b),
                             FloorDivideFloat)(a,b);
}
extern FuncRef ModuloFloat;
extern FuncRef ModuloInt;

template<typename Ta, typename Tb>
auto Modulo(Ta&& a, Tb&& b){
    return resolve_func_impl("Modulo", std::forward<Ta>(a), std::forward<Tb>(b),
                             ModuloFloat, ModuloInt)(a,b);
}

extern FuncRef PowFloat;
extern FuncRef PowInt;

template<typename Ta, typename Tb>
auto Pow(Ta&& a, Tb&& b){
    return resolve_func_impl("Pow", std::forward<Ta>(a), std::forward<Tb>(b),
                             PowFloat, PowInt)(a,b);
}
extern FuncRef NegateInt;
extern FuncRef NegateFloat;

template<typename Ta>
auto Negate(Ta&& a){
    return resolve_func_impl("Negate", std::forward<Ta>(a),
                             NegateFloat, NegateInt)(a);
}

extern FuncRef Concat;
extern FuncRef Slice;



} // NAMESPACE_END(cre)
