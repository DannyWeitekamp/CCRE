#include "../include/py_cre_core.h"
#include "../../include/var.h"
#include "../../include/builtin_funcs.h"
#include "../../include/func.h"


ref<Var> py_Var_ctor(nb::args args, nb::kwargs kwargs) {
    CRE_Type* type = nullptr;
    std::string_view alias;
    bool error = false;
    bool has_type_kwarg = false;
    bool has_alias_kwarg = false;
    
    // Check for kwargs first
    if(kwargs.contains("type")) {
        nb::handle py_type = kwargs["type"];    
        if(nb::isinstance<CRE_Type>(py_type) || py_type.is_type()){
            has_type_kwarg = true;
            type = Type_from_py(py_type);
        } else {
            error = true;
        }
    }
    
    if(kwargs.contains("alias")) {
        nb::handle py_alias = kwargs["alias"];
        if(nb::isinstance<nb::str>(py_alias)) {
            has_alias_kwarg = true;
            alias = nb::cast<std::string_view>(py_alias);
        } else {
            error = true;
        }
    }

    // If both kwargs are provided, use them and ignore positional args
    if(has_type_kwarg && has_alias_kwarg) {
        if(args.size() > 0) {
            throw std::runtime_error("Var constructor: cannot use both keyword arguments and positional arguments");
        }
        if(error) {
            throw std::runtime_error("Var constructor: 'type' must be a Python type or CRE_Type, and 'alias' must be string");
        }
        return new_var(alias, type);
    }

    // If only one kwarg is provided, error
    if(has_type_kwarg || has_alias_kwarg) {
        throw std::runtime_error("Var constructor: must provide both 'type' and 'alias' as keyword arguments, or use positional arguments");
    }

    // Fall back to positional arguments
    if(args.size() != 2) {
        throw std::runtime_error("Var constructor expects exactly 2 arguments (CRE_Type and string), got " + std::to_string(args.size()));
    }

    // Check first argument
    if(nb::isinstance<CRE_Type>(args[0]) || args[0].is_type()) {
        try {
            type = Type_from_py(args[0]);
            if(nb::isinstance<nb::str>(args[1])){
                alias = nb::cast<std::string_view>(args[1]);
            } else {
                error = true;
            }
        } catch(...) {
            error = true;
        }
    } else if(nb::isinstance<nb::str>(args[0])){
        alias = nb::cast<std::string_view>(args[0]);
        if(nb::isinstance<CRE_Type>(args[1]) || args[1].is_type()){
            try {
                type = Type_from_py(args[1]);
            } catch(...) {
                error = true;
            }
        } else {
            error = true;
        }
    } else {
        error = true;
    }

    if(error) throw std::runtime_error("Var constructor: arguments must be either CRE_Type and string, got: " + nb::cast<std::string>(nb::str(args[0])) + " and " + nb::cast<std::string>(nb::str(args[1])));
    
    return new_var(alias, type);
}

// Arithmetic operator helpers that compose functions with Var/Func arguments
ref<Func> py_Var_equals(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Equals->compose(self, other_item);
}

ref<Func> py_Var_not_equals(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Not->compose(Equals->compose(other_item, self));
}

ref<Func> py_Var_add(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Add->compose(self, other_item);
}

ref<Func> py_Var_radd(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Add->compose(other_item, self);
}

ref<Func> py_Var_sub(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Subtract->compose(self, other_item);
}

ref<Func> py_Var_rsub(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Subtract->compose(other_item, self);
}

ref<Func> py_Var_mul(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Multiply->compose(self, other_item);
}

ref<Func> py_Var_rmul(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Multiply->compose(other_item, self);
}

ref<Func> py_Var_truediv(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Divide->compose(self, other_item);
}

ref<Func> py_Var_rtruediv(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Divide->compose(other_item, self);
}

ref<Func> py_Var_mod(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return ModInts->compose(self, other_item);
}

ref<Func> py_Var_rmod(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return ModInts->compose(other_item, self);
}

ref<Func> py_Var_pow(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Pow->compose(self, other_item);
}

ref<Func> py_Var_rpow(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Pow->compose(other_item, self);
}

ref<Func> py_Var_neg(Var* self) {
    return Neg->compose(self);
}


void init_var(nb::module_ & m){
	nb::class_<Var>(m, "Var", nb::type_slots(cre_obj_slots))
    
    // .def(nb::new_(
    //     [](CRE_Type* _type, std::string_view _alias){
    //         return new_var(_alias, _type);
    //     }),
    //     // nb::rv_policy::reference)
    .def(nb::new_(&py_Var_ctor), nb::rv_policy::reference)
    .def("__str__", &Var::to_string)
    .def("__repr__", &Var::to_string)
    .def("__len__", &Var::size)
    // Arithmetic operators
    .def("__add__", &py_Var_add, nb::rv_policy::reference)
    .def("__radd__", &py_Var_radd, nb::rv_policy::reference)
    .def("__sub__", &py_Var_sub, nb::rv_policy::reference)
    .def("__rsub__", &py_Var_rsub, nb::rv_policy::reference)
    .def("__mul__", &py_Var_mul, nb::rv_policy::reference)
    .def("__rmul__", &py_Var_rmul, nb::rv_policy::reference)
    .def("__truediv__", &py_Var_truediv, nb::rv_policy::reference)
    .def("__rtruediv__", &py_Var_rtruediv, nb::rv_policy::reference)
    .def("__mod__", &py_Var_mod, nb::rv_policy::reference)
    .def("__rmod__", &py_Var_rmod, nb::rv_policy::reference)
    .def("__pow__", &py_Var_pow, nb::rv_policy::reference)
    .def("__rpow__", &py_Var_rpow, nb::rv_policy::reference)
    .def("__neg__", &py_Var_neg, nb::rv_policy::reference)
    ;
}
