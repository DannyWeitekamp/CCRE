
#include "../include/py_cre_core.h"
#include "../include/shared_logic_utils.h"
#include "../../include/item.h"
#include "../../include/types.h"
#include "../../include/func.h"
#include "../../include/builtin_funcs.h"
#include "../../include/literal.h"



ref<Func> py_Func([[maybe_unused]] nb::args args) {
    nb::kwargs kwargs; // stub 
    throw std::runtime_error("Not implemented, there is no constructor for Func. Use define_func().");        
    //pass
}

ref<Func> py_define_func([[maybe_unused]] nb::args args) {
    nb::kwargs kwargs; // stub 
        throw std::runtime_error("Not implemented, define_func().");        
    //pass
}



// void* py_resolve_heads(void* dest, nb::object py_obj, CRE_Type* type){
//     if (nb::isinstance<nb::bool_>(py_obj)) {
//         return _copy_convert_from_numerical(dest, nb::cast<bool>(py_obj), type);
//         // return Item(nb::cast<bool>(py_obj));
//     } else if (nb::isinstance<nb::int_>(py_obj)) {
//         return _copy_convert_from_numerical(dest, nb::cast<int64_t>(py_obj), type);
//         // return Item(nb::cast<int>(py_obj));
//     } else if (nb::isinstance<nb::float_>(py_obj)) {
//         return _copy_convert_from_numerical(dest, nb::cast<double>(py_obj), type);
//         // return Item(nb::cast<double>(py_obj));
//     } else if (nb::isinstance<nb::str>(py_obj)) {
//         return _copy_convert_from_str(dest, nb::cast<std::string_view>(py_obj), type);
//         // InternStr intern_str = intern(nb::cast<std::string_view>(py_obj));
//         // cout << "Interned: " << intern_str << endl;
//         // return Item(intern_str);

//     } else if (nb::isinstance<CRE_Obj>(py_obj)) {
//         CRE_Obj* ptr = nb::cast<CRE_Obj*>(py_obj);
//         if(!_check_pointer_is_of_type(ptr, type)) return nullptr;
//         return ptr;
//     }else if (py_obj.is_none()){
//         // return Item();
//         return nullptr;
//     } else {
//         throw std::runtime_error("Func argument type not recognized by CRE: " + nb::cast<std::string>(nb::str(py_obj)));
//         // return Item();
//     }    
// }





nb::object py_Func_call(Func* func, nb::args args, nb::kwargs kwargs) {
    return Item_to_py(py_Func_call_to_item(func, args, kwargs));
}


nb::object py_Func__call__(Func* func, nb::args args, nb::kwargs kwargs) {
    // Check if any argument is Var or Func type
    bool has_var_or_func = false;
    for(size_t i = 0; i < args.size(); ++i){
        if(nb::isinstance<Var>(args[i]) || nb::isinstance<Func>(args[i])){
            has_var_or_func = true;
            break;
        }
    }

    
    // If any argument is Var or Func, call compose
    if(has_var_or_func){
        return nb::cast(py_Func_compose(func, args));
    } else {
        // Otherwise, call the function directly
        return py_Func_call(func, args, kwargs);
    }
}

// Arithmetic operator helpers that compose functions with Func/Func arguments


ref<Literal> py_Func_invert(Func* self) {
    return new_literal(self, true);
}

std::string py_Func_to_string(Func* self, nb::handle verbosity) {
    return self->to_string(nb::cast<uint8_t>(verbosity));
}




void init_func(nb::module_ & m){
	nb::class_<Func, CRE_Obj>(m, "Func", nb::type_slots(cre_obj_slots))
    // .def(nb::new_(&py_Func_ctor), nb::rv_policy::reference)


    // .def("get_refcount", &Fact::get_refcount)

    .def("__str__", &py_Func_to_string, "verbosity"_a=DEFAULT_VERBOSITY)
    .def("__repr__", &py_Func_to_string, "verbosity"_a=DEFAULT_VERBOSITY)
    .def("__call__", &py_Func__call__)
    .def("call", &py_Func_call)
    .def("compose", &py_Func_compose, nb::rv_policy::reference)
    // Arithmetic operators
    .def("__eq__", [](Func* self, nb::handle other){return Equals(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__ne__", [](Func* self, nb::handle other){return new_literal(Equals(self, Item_from_py(other)), true);}, nb::rv_policy::reference)
    .def("__lt__", [](Func* self, nb::handle other){return LessThan(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__gt__", [](Func* self, nb::handle other){return GreaterThan(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__le__", [](Func* self, nb::handle other){return LessThanOrEqual(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__ge__", [](Func* self, nb::handle other){return GreaterThanOrEqual(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__add__", [](Func* self, nb::handle other){return Add(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__radd__", [](Func* self, nb::handle other){return Add(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__sub__", [](Func* self, nb::handle other){return Subtract(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rsub__", [](Func* self, nb::handle other){return Subtract(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__mul__", [](Func* self, nb::handle other){return Multiply(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rmul__", [](Func* self, nb::handle other){return Multiply(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__truediv__", [](Func* self, nb::handle other){return Divide(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rtruediv__", [](Func* self, nb::handle other){return Divide(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__floordiv__", [](Func* self, nb::handle other){return FloorDivide(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rfloordiv__", [](Func* self, nb::handle other){return FloorDivide(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__mod__", [](Func* self, nb::handle other){return Modulo(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rmod__", [](Func* self, nb::handle other){return Modulo(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__pow__", [](Func* self, nb::handle other){return Pow(self,Item_from_py(other));}, nb::rv_policy::reference)
    .def("__rpow__", [](Func* self, nb::handle other){return Pow(Item_from_py(other),self);}, nb::rv_policy::reference)
    .def("__neg__", [](Func* self){return Negate(self);}, nb::rv_policy::reference)
    .def("__invert__", &py_Func_invert, nb::rv_policy::reference)
    

    // .def("__len__", &Fact::size)
    
    // .def("__getitem__", &py_fact_getitem)

    // // .def("__getitem__", &py_fact_getattr)

    // .def("__getattr__", &py_fact_getattr)

    // .def("__setitem__", &py_fact_setitem)
    // .def("__setattr__", &py_fact_setattr)

    // .def("__iter__",  [](Fact* fact) {
    //         return nb::make_iterator(nb::type<Fact>(), "iterator",
    //                                  py_fact_iter_begin(fact),
    //                                  py_fact_iter_end(fact)
    //                                  );
    //     }, nb::keep_alive<0, 1>()//, //nb::rv_policy::reference
    // )
    // .def("iter_items",  [](Fact& fact) {
    //         return nb::make_iterator(nb::type<Fact>(), "iterator",
    //                                  fact.begin(), fact.end());
    //     }, nb::keep_alive<0, 1>()//, //nb::rv_policy::reference
    // )
    ;

    // m.def("NewFact", &NewFact, nb::rv_policy::reference);
    // m.def("iFact", &py_iFact_ctor, nb::rv_policy::reference);
    m.attr("EqualsFloat") = ref<Func>(cre::EqualsFloat);
    m.attr("EqualsInt") = ref<Func>(cre::EqualsInt);
    m.attr("EqualsStr") = ref<Func>(cre::EqualsStr);
    m.attr("And") = ref<Func>(cre::And);
    m.attr("Or") = ref<Func>(cre::Or);
    m.attr("Xor") = ref<Func>(cre::Xor);
    // Skip Not, it is overloaded in py_var.cpp
    // m.attr("Not") = ref<Func>(cre::NotFunc);
    // m.attr("Add") = ref<Func>(cre::Add);
    // m.attr("AddInts") = ref<Func>(cre::AddInts);
    // m.attr("Subtract") = ref<Func>(cre::Subtract);
    // m.attr("SubtractInts") = ref<Func>(cre::SubtractInts);
    // m.attr("Multiply") = ref<Func>(cre::Multiply);
    // m.attr("MultiplyInts") = ref<Func>(cre::MultiplyInts);
    // m.attr("Divide") = ref<Func>(cre::Divide);
    // m.attr("DivideInts") = ref<Func>(cre::DivideInts);
    // m.attr("Modulo") = ref<Func>(cre::Modulo);
    // m.attr("Pow") = ref<Func>(cre::Pow);
    // m.attr("Negate") = ref<Func>(cre::Negate);
    // m.attr("NegateInt") = ref<Func>(cre::NegateInt);
    // m.attr("Concat") = ref<Func>(cre::Concat);
    // m.attr("Slice") = ref<Func>(cre::Slice);
    // m.attr("AddInts") = ref<Func>(cre::AddInts);
    // m.def("And", &cre::And, nb::rv_policy::reference);
    // m.def("Add", &cre::Add, nb::rv_policy::reference);
    // m.def("AddInts", &cre::AddInts, nb::rv_policy::reference);
    


}
