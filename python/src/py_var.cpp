#include "../include/py_cre_core.h"
#include "../include/shared_logic_utils.h"
#include "../../include/var.h"
#include "../../include/builtin_funcs.h"
#include "../../include/func.h"
#include "../../include/literal.h"
#include "../../include/ref.h"

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
    if(var->alias.get_t_id() != T_ID_UNDEF){
        return;
    }

    PyObject* py_proxy = (PyObject*) var->get_proxy_obj();

    std::string_view alias_str = "";
    if(alias_str == "" && py_proxy != nullptr){
        // Search for proxy in locals
        nb::handle locals_handle = PyEval_GetLocals();
        // nb::dict locals = nb::cast<nb::dict>(locals_handle);
        alias_str = locate_py_name_fast(py_proxy, locals_handle.ptr());

        if(alias_str == ""){
            nb::handle globals_handle = PyEval_GetGlobals();
            alias_str = locate_py_name_fast(py_proxy, globals_handle.ptr());    
        }

        if(alias_str != ""){
            var->alias = Item(intern(alias_str));
            // cout << "Var found: " << var->get_alias_str() << endl;
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


ref<Var> _py_Var_new_or_locate_self(Item alias, CRE_Type* type, uint8_t kind=VAR_KIND_ABSOLUTE){
    std::string key = fmt::format("__CRE_Var_{}", alias.as<std::string>());
    nb::str py_key = nb::str(key.c_str());
    nb::dict locals = nb::cast<nb::dict>((nb::handle) PyEval_GetLocals());
    nb::handle result = locals.get(py_key, nb::none());
    ref<Var> self = nullptr;
    if(!result.is_none()){
        self = nb::cast<Var*>(result);

        if(type == cre_undef ||
           (self->base_type == type && self->kind == kind)){
            return self;    
        }
            // throw std::domain_error(
            //     fmt::format("Different types or kinds for Var '{0}' in expression. "
            //        "Definition of {1} != Var({2},{0}).", alias.as<std::string>(), self->repr(), type->to_string())
            // );
    }
        
    // If cannot reuse Var then make a new one 
    self = new_var(alias, type, kind);
    locals[py_key] = nb::cast(self);
    
    return self;
}

ref<Var> _py_Var_ctor(nb::args args, nb::kwargs kwargs, uint8_t kind=VAR_KIND_ABSOLUTE) {
    CRE_Type* type = cre_undef;
    Item alias = Item();
    Item fact = Item();
    bool error = false;
    bool has_type_kwarg = false;
    bool has_alias_kwarg = false;

    // Resolve kwargs first
    nb::handle py_type = kwargs.get("type", nb::none());
    nb::handle py_alias = kwargs.get("alias", nb::none());

    // Try args next
    if(args.size() > 0){
        if(nb::isinstance<CRE_Type>(args[0]) || args[0].is_type()){
            if(py_type.is_none()) py_type = args[0];
        }else if(nb::isinstance<nb::str>(args[0]) || 
                 nb::isinstance<nb::int_>(args[0])){
            if(py_alias.is_none()) py_alias = args[0];
        }else if(nb::isinstance<Fact>(args[0])){
            fact = Item_from_py(args[0]);
        }else{
            error = true;
        }
        if(args.size() > 1){
            if(nb::isinstance<CRE_Type>(args[1]) || args[1].is_type()){
                if(py_type.is_none()) py_type = args[1];
            }else if(nb::isinstance<nb::str>(args[1]) || 
                     nb::isinstance<nb::int_>(args[1])){
                if(py_alias.is_none()) py_alias = args[1];
            }else{
                error = true;
            }
        }
    }

    // Resolve alias
    if(nb::isinstance<nb::str>(py_alias)){
        alias = intern(nb::cast<std::string_view>(py_alias));
    }else if(nb::isinstance<nb::int_>(py_alias)){
        alias = nb::cast<int64_t>(py_alias);
    }

    // Resolve type
    if(!py_type.is_none()){
        try {
            type = Type_from_py(py_type);
        } catch(...) {
            error = true;
        }
    }

    if(error){
        throw std::invalid_argument("Invalid arguments for Var(type, alias) constructor:"
            " 'type' must be a Python type or CRE_Type, and 'alias' must be string or integer.");   
    }

    ref<Var> var = new_var(alias, type, kind);

    if(!fact.is_undef()){
        ref<Logic> fact_conj = fact_to_conjunct(fact.as<Fact*>());
        var->bound_obj = Item(fact_conj);
    }
    return var;
    // // Check for kwargs first
    // if(kwargs.contains("type")) {
    //     nb::handle py_type = kwargs["type"];    
    //     if(nb::isinstance<CRE_Type>(py_type) || py_type.is_type()){
    //         has_type_kwarg = true;
    //         type = Type_from_py(py_type);
    //     } else {
    //         error = true;
    //     }
    // }
    
    // if(kwargs.contains("alias")) {
    //     nb::handle py_alias = kwargs["alias"];
    //     if(nb::isinstance<nb::str>(py_alias)) {
    //         has_alias_kwarg = true;
    //         alias = nb::cast<std::string_view>(py_alias);
    //     }else if(nb::isinstance<nb::int_>(py_alias)){
    //         alias = nb::cast<int64_t>(py_alias);
    //     } else {
    //         error = true;
    //     }
    // }

    

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

    // // Check first argument
    // if(args.size() > 0){
    //     if(nb::isinstance<CRE_Type>(args[0]) || args[0].is_type()) {
    //         try {
    //             if(!has_type_kwarg) type = Type_from_py(args[0]);                    
    //             if(args.size()==2){
    //                 if(nb::isinstance<nb::str>(args[1])){
    //                     if(!has_alias_kwarg) alias = intern(nb::cast<std::string_view>(args[1]));
    //                 }else if(nb::isinstance<nb::int_>(args[1])){
    //                     if(!has_alias_kwarg) alias = nb::cast<int64_t>(args[1]);
    //                 } else {
    //                     error = true;
    //                 }    
    //             }
    //         } catch(...) {
    //             error = true;
    //         }
    //     } else if(nb::isinstance<nb::str>(args[0]) || nb::isinstance<nb::int_>(args[0])){
    //         if(!has_alias_kwarg){
    //             if(nb::isinstance<nb::str>(args[0])){
    //                 alias = intern(nb::cast<std::string_view>(args[0]));
    //             }else if(nb::isinstance<nb::int_>(args[0])){
    //                 alias = nb::cast<int64_t>(args[0]);
    //             }
    //         }
    //         if(args.size()==2){
    //             if(nb::isinstance<CRE_Type>(args[1]) || args[1].is_type()){
    //                 try {
    //                     if(!has_type_kwarg) type = Type_from_py(args[1]);
    //                 } catch(...) {
    //                     error = true;
    //                 }    
    //             } else {
    //                 error = true;
    //             }
    //         }
    //     } else {
    //         error = true;
    //     }
    // }

    // if(error) throw std::invalid_argument("Var constructor: optional positional arguments must be CRE_Type for 'type' and string for 'alias'");

    
    // if(self != nullptr) return self;       
    // return new_var(alias, type, kind);
}

ref<Var> py_Var_ctor(nb::args args, nb::kwargs kwargs) {
    return _py_Var_ctor(args, kwargs);
}

ref<Var> py_Var_getattr(Var* self, std::string_view attr) {
    return self->extend_attr(attr);
}

ref<Var> py_Var_getitem(Var* self, nb::handle attr) {
    if(nb::isinstance<nb::int_>(attr)){
        return self->extend_item(nb::cast<int64_t>(attr));
    } else if(nb::isinstance<nb::str>(attr)){
        return self->extend_attr(nb::cast<std::string_view>(attr));
    }else{
        throw std::invalid_argument("Invalid argument for Var.__getitem__: must be integer or string");
    }
}

nb::object py_Not(nb::args args, nb::kwargs kwargs) {
    if(args.size() == 1) {
        Item arg_item = Item_from_py(args[0]);
        if(arg_item.is_evaluatable()) {
            return nb::cast(cre::NotFunc->compose(arg_item));
        }
    }
    return nb::cast(_py_Var_ctor(args, kwargs, VAR_KIND_NOT));
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

	nb::class_<Var, CRE_Obj>(m, "Var", nb::type_slots(cre_obj_slots))
    
    // .def(nb::new_(
    //     [](CRE_Type* _type, std::string_view _alias){
    //         return new_var(_alias, _type);
    //     }),
    //     // nb::rv_policy::reference)
    .def(nb::new_(&py_Var_ctor), nb::rv_policy::reference)
    .def("__str__", &Var::to_string)
    .def("__repr__", [](Var* self){return self->repr(true);}, nb::rv_policy::reference)
    .def("__len__", &Var::size)
    .def("__getattr__", &py_Var_getattr)
    .def("__getitem__", &py_Var_getitem)
    // Arithmetic operators
    .def("__eq__", [](Var* self, nb::handle other){
        if(nb::isinstance<Fact>(other)){
            Fact* fact = nb::cast<Fact*>(other);
            return nb::cast(fact_to_conjunct(fact, self));
        }else{
            return nb::cast(py_cre_equals(self,other));
        }
    }, nb::rv_policy::reference)
    .def("__ne__", [](Var* self, nb::handle other){return py_cre_not_equals(self,other);}, nb::rv_policy::reference)
    .def("__lt__", [](Var* self, nb::handle other){return py_cre_less_than(self,other);}, nb::rv_policy::reference)
    .def("__gt__", [](Var* self, nb::handle other){return py_cre_greater_than(self,other);}, nb::rv_policy::reference)
    .def("__le__", [](Var* self, nb::handle other){return py_cre_less_than_or_equal(self,other);}, nb::rv_policy::reference)
    .def("__ge__", [](Var* self, nb::handle other){return py_cre_greater_than_or_equal(self,other);}, nb::rv_policy::reference)
    .def("__add__", [](Var* self, nb::handle other){return py_cre_add(self,other);}, nb::rv_policy::reference)
    .def("__radd__", [](Var* self, nb::handle other){return py_cre_radd(self,other);}, nb::rv_policy::reference)
    .def("__sub__", [](Var* self, nb::handle other){return py_cre_sub(self,other);}, nb::rv_policy::reference)
    .def("__rsub__", [](Var* self, nb::handle other){return py_cre_rsub(self,other);}, nb::rv_policy::reference)
    .def("__mul__", [](Var* self, nb::handle other){return py_cre_mul(self,other);}, nb::rv_policy::reference)
    .def("__rmul__", [](Var* self, nb::handle other){return py_cre_rmul(self,other);}, nb::rv_policy::reference)
    .def("__truediv__", [](Var* self, nb::handle other){return py_cre_truediv(self,other);}, nb::rv_policy::reference)
    .def("__rtruediv__", [](Var* self, nb::handle other){return py_cre_rtruediv(self,other);}, nb::rv_policy::reference)
    .def("__floordiv__", [](Var* self, nb::handle other){return py_cre_floordiv(self,other);}, nb::rv_policy::reference)
    .def("__rfloordiv__", [](Var* self, nb::handle other){return py_cre_rfloordiv(self,other);}, nb::rv_policy::reference)
    .def("__mod__", [](Var* self, nb::handle other){return py_cre_mod(self,other);}, nb::rv_policy::reference)
    .def("__rmod__", [](Var* self, nb::handle other){return py_cre_rmod(self,other);}, nb::rv_policy::reference)
    .def("__pow__", [](Var* self, nb::handle other){return py_cre_pow(self,other);}, nb::rv_policy::reference)
    .def("__rpow__", [](Var* self, nb::handle other){return py_cre_rpow(self,other);}, nb::rv_policy::reference)
    .def("__neg__", [](Var* self){return py_cre_neg(self);}, nb::rv_policy::reference)
    ;

    // m.def("peak_locals", &peak_locals);
    m.def("resolve_alias", &py_Var_locate_alias);
    m.def("resolve_alias_fast", &py_Var_locate_alias_fast);
    m.def("do_nothing", &do_nothing);
    m.def("Not", &py_Not, nb::rv_policy::reference);
    m.def("Exists", [](nb::args args, nb::kwargs kwargs){return _py_Var_ctor(args, kwargs, VAR_KIND_EXIST);}, nb::rv_policy::reference);
    m.def("Bound", [](nb::args args, nb::kwargs kwargs){return _py_Var_ctor(args, kwargs, VAR_KIND_BOUND);}, nb::rv_policy::reference);
    m.def("Opt", [](nb::args args, nb::kwargs kwargs){return _py_Var_ctor(args, kwargs, VAR_KIND_OPTIONAL);}, nb::rv_policy::reference);
}
