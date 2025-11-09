#include "../include/py_cre_core.h"
#include "../include/shared_func_utils.h"
#include "../../include/logic.h"
#include "../../include/func.h"
#include "../../include/builtin_funcs.h"


// Constructor for Logic that takes a kind and variadic args
ref<Logic> py_Logic_ctor(uint8_t kind, nb::args args) {
    ref<Logic> logic = new_logic(kind);
    
    // Convert all arguments to CRE_Obj* and insert them
    for (auto it = args.begin(); it != args.end(); ++it) {
        CRE_Obj* obj = py_to_cre_obj(*it);
        logic->_insert_arg(obj);
    }
    
    logic->_finalize();
    return logic;
}

// Helper function for AND
ref<Logic> py_AND(nb::args args) {
    ref<Logic> logic = new_logic(CONDS_KIND_AND);
    
    for (auto it = args.begin(); it != args.end(); ++it) {
        CRE_Obj* obj = py_to_cre_obj(*it);
        logic->_insert_arg(obj);
    }
    
    logic->_finalize();
    return logic;
}

// Helper function for OR
ref<Logic> py_OR(nb::args args) {
    ref<Logic> logic = new_logic(CONDS_KIND_OR);
    
    for (auto it = args.begin(); it != args.end(); ++it) {
        CRE_Obj* obj = py_to_cre_obj(*it);
        logic->_insert_arg(obj);
    }
    
    logic->_finalize();
    return logic;
}

// Implementation of __call__ for Literal
nb::object py_Literal__call__(Literal* lit, nb::args args, nb::kwargs kwargs) {
    // Only Func literals can be called
    if(lit->kind != LIT_KIND_FUNC){
        throw std::runtime_error("Literal.__call__: Only Func literals can be called, got kind: " + 
                                std::to_string(lit->kind));
    }
    
    // Cast obj to Func* 
    Func* func = (Func*) lit->obj.get();
    
    // Check if any argument is Var or Func type (for composition)
    bool has_var_or_func = false;
    for(size_t i = 0; i < args.size(); ++i){
        if(nb::isinstance<Var>(args[i]) || nb::isinstance<Func>(args[i])){
            has_var_or_func = true;
            break;
        }
    }

    // If any argument is Var or Func, call compose
     if(has_var_or_func){
        if(lit->negated){
            return nb::cast(Negate->compose(py_Func_compose(func, args)));
        } else {
            return nb::cast(py_Func_compose(func, args));
        }
    // Otherwise, call the function directly
    } else {
        Item item = py_Func_call_to_item(func, args, kwargs);
        if(lit->negated) item = Item(!item.as<bool>());
            
        return Item_to_py(item);
    }
}

void init_logic(nb::module_ & m) {

    cout << "INIT LOGIC" << endl;
    // Expose constants
    m.attr("CONDS_KIND_AND") = CONDS_KIND_AND;
    m.attr("CONDS_KIND_OR") = CONDS_KIND_OR;

    // Define Literal class
    nb::class_<Literal, CRE_Obj>(m, "Literal", nb::type_slots(cre_obj_slots))
        .def("__str__", &Literal::to_string, "verbosity"_a=DEFAULT_VERBOSITY)
        .def("__repr__", &Literal::to_string, "verbosity"_a=DEFAULT_VERBOSITY)
        .def("to_string", &Literal::to_string)
        .def("__call__", &py_Literal__call__)
        .def_ro("negated", &Literal::negated)
        .def_ro("kind", &Literal::kind)
        .def_ro("obj", &Literal::obj)
        .def_ro("vars", &Literal::vars)
        .def_ro("var_inds", &Literal::var_inds) 
        ;
        
    // Define Logic class
    nb::class_<Logic, CRE_Obj>(m, "Logic", nb::type_slots(cre_obj_slots))
        .def(nb::new_(&py_Logic_ctor), "kind"_a, "args"_a, nb::rv_policy::reference)
        .def("__str__", &Logic::to_string)
        .def("__repr__", &Logic::to_string)
        .def("to_string", &Logic::to_string)
        .def("basic_str", &Logic::basic_str)
        .def("standard_str", [](Logic* self, std::string_view indent) {
            return self->standard_str(indent, nullptr);
        }, "indent"_a = "  ")
        .def_ro("kind", &Logic::kind)
        .def_ro("n_abs_vars", &Logic::n_abs_vars)
        .def("__len__", [](Logic* self) { return self->items.size(); })
        ;
    
    // Expose AND and OR as module-level functions
    m.def("AND", &py_AND, "args"_a, nb::rv_policy::reference);
    m.def("OR", &py_OR, "args"_a, nb::rv_policy::reference);
}

