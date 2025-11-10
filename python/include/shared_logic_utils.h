#include "../include/py_cre_core.h"
#include "../include/builtin_funcs.h"



Item py_Func_call_to_item(Func* func, nb::args args, nb::kwargs kwargs);
ref<Func> py_Func_compose(Func* func, nb::args args);
CRE_Obj* py_to_cre_obj(nb::handle py_obj);
void* py_resolve_heads(void* dest, nb::object py_obj, const HeadInfo& hi);



template<typename T>
uint16_t get_obj_t_id(T* obj) {
    if constexpr(std::is_same_v<T, Var>){
        return obj->head_type->get_t_id();
    } else if constexpr(std::is_same_v<T, Func>){
        return obj->return_type->get_t_id();
    } else if constexpr(std::is_same_v<T, Literal>){
        if(obj->is_func()){
            return ((Func*) obj->obj.get())->return_type->get_t_id();
        } else {
            throw std::runtime_error("Literal is not a Func");
        }
    }
    throw std::runtime_error("Unsupported object type");
    return T_ID_UNDEF;
}

// Arithmetic operator helpers that compose functions with Var/Func arguments
template<typename T>
ref<Func> py_cre_equals(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Equals->compose(self, other_item);
}

template<typename T>
ref<Literal> py_cre_not_equals(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return new_literal(Equals->compose(self, other_item), true);
}

template<typename T>
ref<Func> py_cre_less_than(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return LessThan->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_greater_than(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return GreaterThan->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_less_than_or_equal(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return LessThanOrEqual->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_greater_than_or_equal(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return GreaterThanOrEqual->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_add(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    cout << "Add: " << self->to_string() << " + " << other_item.to_string() << " Self t_id: " << get_obj_t_id(self) << " Other t_id: " << other_item.eval_t_id() << endl;
    if(get_obj_t_id(self) == T_ID_STR && other_item.eval_t_id() == T_ID_STR){
        cout << "CONCAT!!" << endl;
        return Concat->compose(self, other_item);
    }else{
        return Add->compose(self, other_item);
    }
    
}

template<typename T>
ref<Func> py_cre_radd(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Add->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_sub(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Subtract->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_rsub(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Subtract->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_mul(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Multiply->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_rmul(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Multiply->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_truediv(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Divide->compose(self, other_item);
    // if(get_obj_t_id(self) == T_ID_INT && other_item.eval_t_id() == T_ID_INT){
    //     cout << "DivideInts: " << self->to_string() << " / " << other_item.to_string() << " Other t_id: " << other_item.eval_t_id() << endl;
    //     return DivideInts->compose(self, other_item);
    // }else{
    //     cout << "Divide: " << self->to_string() << " / " << other_item.to_string() << " Other t_id: " << other_item.eval_t_id() << endl;
    //     return Divide->compose(self, other_item);
    // }
}

template<typename T>
ref<Func> py_cre_rtruediv(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Divide->compose(other_item, self);
    // if(get_obj_t_id(self) == T_ID_INT && other_item.eval_t_id() == T_ID_INT){
    //     return DivideInts->compose(other_item, self);
    // }else{
    //     return Divide->compose(other_item, self);
    // }
}

template<typename T>
ref<Func> py_cre_floordiv(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return FloorDivide->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_rfloordiv(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return FloorDivide->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_mod(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Modulo->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_rmod(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Modulo->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_pow(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Pow->compose(self, other_item);
}

template<typename T>
ref<Func> py_cre_rpow(T* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Pow->compose(other_item, self);
}

template<typename T>
ref<Func> py_cre_neg(T* self) {
    return Negate->compose(self);
}