#include "../include/py_cre_core.h"
#include "../../include/var.h"
#include "../../include/builtin_funcs.h"
#include "../../include/func.h"
#include "../../include/literal.h"


void do_nothing(){

}


std::string_view locate_py_name(PyObject* py_proxy, nb::handle dict){
    nb::dict locals = nb::cast<nb::dict>(dict);
    for (auto [key, value] : locals) {
        if(value.ptr() == py_proxy){
            return nb::cast<std::string_view>(nb::handle(key));
        }
    }
    return "";
}

//  Use internal Python API for looping over dicts to avoid increfing each handle
std::string_view locate_py_name_fast(PyObject* py_proxy, PyObject* dict){
    Py_ssize_t pos = 0;
    PyObject *key = nullptr;
    PyObject *value = nullptr;
    // cout << "LOOKING FOR" << uint64_t(py_proxy) << endl;
    while(PyDict_Next(dict, &pos, &key, &value) != 0){
        // cout << "pos=" << pos << " value=" << uint64_t(value) << endl; 
        if(value == py_proxy){
            return nb::cast<std::string_view>(nb::handle(key));
        }
    }
    return "";
}




// This is a 
void py_Var_locate_alias_fast(Var* var){
    if(var->alias != ""){
        return;
    }

    PyObject* py_proxy = (PyObject*) var->get_proxy_obj();

    std::string_view alias = "";
    if(alias == "" && py_proxy != nullptr){
        // Search for proxy in locals
        nb::handle locals_handle = PyEval_GetLocals();
        // nb::dict locals = nb::cast<nb::dict>(locals_handle);
        alias = locate_py_name_fast(py_proxy, locals_handle.ptr());

        if(alias == ""){
            nb::handle globals_handle = PyEval_GetGlobals();
            alias = locate_py_name_fast(py_proxy, globals_handle.ptr());    
        }

        if(alias != ""){
            var->alias = Item(intern(alias));
            // cout << "Var found: " << alias << endl;
            return;
        }

        
        // nb::dict globals = nb::cast<nb::dict>(globals_handle);
        // for (auto [k, v] : globals) {
        //     if(v.ptr() == py_proxy){
        //         alias = nb::cast<std::string_view>(k);
        //         cout << "Var found: " << nb::cast<std::string>(nb::str(k)) << endl;
        //         return;
        //         // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        //     }
        //     // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        // }    

        // cout << "Alias not found!" << endl;
    }else{
        cout << "NO PROXY" << endl;
    }
}

void py_Var_locate_alias(Var* var){
    if(var->alias != ""){
        return;
    }

    PyObject* py_proxy = (PyObject*) var->get_proxy_obj();

    std::string_view alias = "";
    if(py_proxy != nullptr){
        // Search for proxy in locals
        nb::handle locals_handle = PyEval_GetLocals();
        alias = locate_py_name(py_proxy, locals_handle);

        // Search for proxy in globals
        if(alias == ""){
            nb::handle globals_handle = PyEval_GetGlobals();
            alias = locate_py_name(py_proxy, globals_handle);
        }

        if(alias != ""){
            // cout << "Var found: " << alias << endl;
            var->alias = Item(intern(alias));
            return;
        }


        // nb::handle locals_handle = PyEval_GetLocals();
        // nb::dict locals = nb::cast<nb::dict>(locals_handle);
        // for (auto [k, v] : locals) {
        //     if(v.ptr() == py_proxy){
        //         alias = nb::cast<std::string_view>(k);
        //         // cout << "Var found: " << nb::cast<std::string>(nb::str(k)) << endl;
        //         return;
        //         // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        //     }
        //     // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        // }    

        // nb::handle globals_handle = PyEval_GetGlobals();
        // nb::dict globals = nb::cast<nb::dict>(globals_handle);
        // for (auto [k, v] : globals) {
        //     if(v.ptr() == py_proxy){
        //         alias = nb::cast<std::string_view>(k);
        //         // cout << "Var found: " << nb::cast<std::string>(nb::str(k)) << endl;
        //         return;
        //         // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        //     }
        //     // cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
        // }    

        // cout << "Alias not found!" << endl;
    }else{
        cout << "NO PROXY" << endl;
    }
}

ref<Var> py_Var_ctor(nb::args args, nb::kwargs kwargs) {
    CRE_Type* type = cre_undef;
    std::string_view alias = "";
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

    if(error){
        throw std::invalid_argument("Var constructor: 'type' must be a Python type or CRE_Type, and 'alias' must be string");    
    }

    // // If both kwargs are provided, use them and ignore positional args
    // if(has_type_kwarg && has_alias_kwarg) {
    //     if(args.size() > 0) {
    //         throw std::runtime_error("Var constructor: cannot use both keyword arguments and positional arguments");
    //     }
    //     if(error) {
    //         throw std::runtime_error("Var constructor: 'type' must be a Python type or CRE_Type, and 'alias' must be string");
    //     }
    //     return new_var(alias, type);
    // }

    // // If only one kwarg is provided, error
    // if(has_type_kwarg || has_alias_kwarg) {
    //     throw std::runtime_error("Var constructor: must provide both 'type' and 'alias' as keyword arguments, or use positional arguments");
    // }

    // // Fall back to positional arguments
    // if(args.size() != 2) {
    //     throw std::runtime_error("Var constructor expects exactly 2 arguments (CRE_Type and string), got " + std::to_string(args.size()));
    // }

    // Check first argument
    if(args.size() > 0){
        if(nb::isinstance<CRE_Type>(args[0]) || args[0].is_type()) {
            try {
                if(!has_type_kwarg) type = Type_from_py(args[0]);                    
                if(args.size()==2){
                    if(nb::isinstance<nb::str>(args[1])){
                        if(!has_alias_kwarg) alias = nb::cast<std::string_view>(args[1]);
                    } else {
                        error = true;
                    }    
                }
            } catch(...) {
                error = true;
            }
        } else if(nb::isinstance<nb::str>(args[0])){
            if(!has_alias_kwarg) alias = nb::cast<std::string_view>(args[0]);
            if(args.size()==2){
                if(nb::isinstance<CRE_Type>(args[1]) || args[1].is_type()){
                    try {
                        if(!has_type_kwarg) type = Type_from_py(args[1]);
                    } catch(...) {
                        error = true;
                    }    
                } else {
                    error = true;
                }
            }
        } else {
            error = true;
        }
    }

    if(error) throw std::invalid_argument("Var constructor: optional positional arguments must be CRE_Type for 'type' and string for 'alias'");
    
    return new_var(alias, type);
}

// Arithmetic operator helpers that compose functions with Var/Func arguments
ref<Func> py_Var_equals(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Equals->compose(self, other_item);
}

ref<Literal> py_Var_not_equals(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return new_literal(Equals->compose(self, other_item), true);
}

ref<Func> py_Var_less_than(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return LessThan->compose(self, other_item);
}

ref<Func> py_Var_greater_than(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return GreaterThan->compose(self, other_item);
}

ref<Func> py_Var_less_than_or_equal(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return LessThanOrEqual->compose(self, other_item);
}

ref<Func> py_Var_greater_than_or_equal(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return GreaterThanOrEqual->compose(self, other_item);
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

ref<Func> py_Var_floordiv(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return FloorDivide->compose(self, other_item);
}

ref<Func> py_Var_rfloordiv(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return FloorDivide->compose(other_item, self);
}

ref<Func> py_Var_mod(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Modulo->compose(self, other_item);
}

ref<Func> py_Var_rmod(Var* self, nb::handle other) {
    Item other_item = Item_from_py(other);
    return Modulo->compose(other_item, self);
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
    return Negate->compose(self);
}




void peak_locals(){
    PyObject* py_locals = PyEval_GetLocals();
    nb::handle locals_handle(py_locals);
    
    if (!nb::isinstance<nb::dict>(locals_handle)) {
        cout << "PyEval_GetLocals() did not return a dict" << endl;
        return;
    }
    
    nb::dict locals = nb::cast<nb::dict>(locals_handle);
    
    for (auto [k, v] : locals) {
        cout << "Key: " << nb::cast<std::string>(nb::str(k)) << ", Value: " << nb::cast<std::string>(nb::str(v)) << endl;
    }
}


void init_var(nb::module_ & m){

    // Register the Python implementation of locate_var_alias
    ext_locate_var_alias = &py_Var_locate_alias_fast;

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
    .def("__eq__", &py_Var_equals, nb::rv_policy::reference)
    .def("__ne__", &py_Var_not_equals, nb::rv_policy::reference)
    .def("__lt__", &py_Var_less_than, nb::rv_policy::reference)
    .def("__gt__", &py_Var_greater_than, nb::rv_policy::reference)
    .def("__le__", &py_Var_less_than_or_equal, nb::rv_policy::reference)
    .def("__ge__", &py_Var_greater_than_or_equal, nb::rv_policy::reference)
    .def("__add__", &py_Var_add, nb::rv_policy::reference)
    .def("__radd__", &py_Var_radd, nb::rv_policy::reference)
    .def("__sub__", &py_Var_sub, nb::rv_policy::reference)
    .def("__rsub__", &py_Var_rsub, nb::rv_policy::reference)
    .def("__mul__", &py_Var_mul, nb::rv_policy::reference)
    .def("__rmul__", &py_Var_rmul, nb::rv_policy::reference)
    .def("__truediv__", &py_Var_truediv, nb::rv_policy::reference)
    .def("__rtruediv__", &py_Var_rtruediv, nb::rv_policy::reference)
    .def("__floordiv__", &py_Var_floordiv, nb::rv_policy::reference)
    .def("__rfloordiv__", &py_Var_rfloordiv, nb::rv_policy::reference)
    .def("__mod__", &py_Var_mod, nb::rv_policy::reference)
    .def("__rmod__", &py_Var_rmod, nb::rv_policy::reference)
    .def("__pow__", &py_Var_pow, nb::rv_policy::reference)
    .def("__rpow__", &py_Var_rpow, nb::rv_policy::reference)
    .def("__neg__", &py_Var_neg, nb::rv_policy::reference)
    ;

    // m.def("peak_locals", &peak_locals);
    m.def("resolve_alias", &py_Var_locate_alias);
    m.def("resolve_alias_fast", &py_Var_locate_alias_fast);
    m.def("do_nothing", &do_nothing);
}
