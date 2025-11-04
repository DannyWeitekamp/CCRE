#include "../include/py_cre_core.h"
#include "../../include/var.h"


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


void init_logical(nb::module_ & m){
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
    ;
}
