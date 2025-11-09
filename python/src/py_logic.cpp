#include "../include/py_cre_core.h"
#include "../../include/logic.h"

// Helper function to convert Python object to CRE_Obj*
CRE_Obj* py_to_cre_obj(nb::handle py_obj) {
    // Try to cast to various CRE_Obj types
    if (nb::isinstance<Var>(py_obj)) {
        return nb::cast<Var*>(py_obj);
    } else if (nb::isinstance<Func>(py_obj)) {
        return nb::cast<Func*>(py_obj);
    } else if (nb::isinstance<Fact>(py_obj)) {
        return nb::cast<Fact*>(py_obj);
    } else if (nb::isinstance<Logic>(py_obj)) {
        return nb::cast<Logic*>(py_obj);
    } else if (nb::isinstance<CRE_Obj>(py_obj)) {
        return nb::cast<CRE_Obj*>(py_obj);
    } else {
        // For other types (int, float, str, etc.), we need to create a literal
        // But Logic::_insert_arg handles this by calling new_literal
        // So we should convert to Item first, but _insert_arg expects CRE_Obj*
        // Actually, looking at the code, _insert_arg handles T_ID_FUNC and T_ID_FACT
        // by creating literals. For primitive types, we'd need to handle them differently.
        // For now, throw an error for unsupported types
        throw std::runtime_error("Logic argument must be Var, Func, Fact, or Logic, got: " + 
                                nb::cast<std::string>(nb::str(py_obj.type())));
    }
}

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

void init_logic(nb::module_ & m) {

    cout << "INIT LOGIC" << endl;
    // Expose constants
    m.attr("CONDS_KIND_AND") = CONDS_KIND_AND;
    m.attr("CONDS_KIND_OR") = CONDS_KIND_OR;
    
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

