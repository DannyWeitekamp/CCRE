#include "../include/py_cre_core.h"
#include "../include/shared_logic_utils.h"
#include "../../include/logic.h"
#include "../../include/func.h"
#include "../../include/builtin_funcs.h"
#include "../../include/structure_map.h"


// Constructor for Logic that takes a kind and variadic args
ref<Logic> py_Logic_ctor(uint8_t kind, nb::args args) {
    ref<Logic> logic = new_logic(kind);
    
    // Convert all arguments to CRE_Obj* and insert them
    for (auto it = args.begin(); it != args.end(); ++it) {
        Item obj = Item_from_py(*it);
        logic->_insert_arg(obj);
    }
    
    logic->_finalize();
    return logic;
}

// Helper function for AND
ref<Logic> py_AND(nb::args args) {
    ref<Logic> logic = new_logic(CONDS_KIND_AND);
    
    for (auto it = args.begin(); it != args.end(); ++it) {
        Item obj = Item_from_py(*it);
        logic->_insert_arg(obj);
    }
    
    logic->_finalize();
    return logic;
}

// Helper function for OR
ref<Logic> py_OR(nb::args args) {
    ref<Logic> logic = new_logic(CONDS_KIND_OR);
    
    for (auto it = args.begin(); it != args.end(); ++it) {
        Item obj = Item_from_py(*it);
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


ref<Logic> py_Logic_antiunify(
    Logic* self, Logic* other, 
    nb::object fixed_inds = nb::none(),
    bool return_score = false, 
    bool fix_same_var = false) {
        
    std::vector<int16_t>* fixed_inds_ptr = nullptr;
    std::vector<int16_t> fixed_inds_vec;
    
    if (fixed_inds.is_valid() && !fixed_inds.is_none()) {
        // Convert Python iterable to vector
        // Handle different iterable types (list, tuple, generator, etc.)
        if (nb::isinstance<nb::list>(fixed_inds)) {
            nb::list py_list = nb::cast<nb::list>(fixed_inds);
            fixed_inds_vec.reserve(py_list.size());
            for (auto item : py_list) {
                fixed_inds_vec.push_back(nb::cast<int16_t>(item));
            }
        } else if (nb::isinstance<nb::tuple>(fixed_inds)) {
            nb::tuple py_tuple = nb::cast<nb::tuple>(fixed_inds);
            fixed_inds_vec.reserve(py_tuple.size());
            for (auto item : py_tuple) {
                fixed_inds_vec.push_back(nb::cast<int16_t>(item));
            }
        } else {
            // For other iterables (generators, custom iterables, etc.), use Python's iter()
            nb::object iter_obj = nb::steal(PyObject_GetIter(fixed_inds.ptr()));
            if (!iter_obj.is_valid()) {
                throw std::runtime_error("fixed_inds must be an iterable of integers");
            }
            while (true) {
                nb::object item = nb::steal(PyIter_Next(iter_obj.ptr()));
                if (!item.is_valid()) {
                    if (PyErr_Occurred() && PyErr_ExceptionMatches(PyExc_StopIteration)) {
                        PyErr_Clear();
                    }
                    break;
                }
                fixed_inds_vec.push_back(nb::cast<int16_t>(item));
            }
        }
        fixed_inds_ptr = &fixed_inds_vec;
    }
    
    auto result = antiunify_logic(self, other, fixed_inds_ptr, return_score, fix_same_var);
    // cout << "ANTIUNIFY: " << result->get_refcount() << " " << self->get_refcount() << " " << other->get_refcount() << endl;
    return result;
}


ref<Literal> py_literal_invert(Literal* lit) {
    return new_literal(lit->obj.get(), !lit->negated);
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

        .def("__eq__", [](Literal* self, nb::handle other){return py_cre_equals(self,other);}, nb::rv_policy::reference)
        .def("__ne__", [](Literal* self, nb::handle other){return py_cre_not_equals(self,other);}, nb::rv_policy::reference)
        .def("__lt__", [](Literal* self, nb::handle other){return py_cre_less_than(self,other);}, nb::rv_policy::reference)
        .def("__gt__", [](Literal* self, nb::handle other){return py_cre_greater_than(self,other);}, nb::rv_policy::reference)
        .def("__le__", [](Literal* self, nb::handle other){return py_cre_less_than_or_equal(self,other);}, nb::rv_policy::reference)
        .def("__ge__", [](Literal* self, nb::handle other){return py_cre_greater_than_or_equal(self,other);}, nb::rv_policy::reference)
        .def("__add__", [](Literal* self, nb::handle other){return py_cre_add(self,other);}, nb::rv_policy::reference)
        .def("__radd__", [](Literal* self, nb::handle other){return py_cre_radd(self,other);}, nb::rv_policy::reference)
        .def("__sub__", [](Literal* self, nb::handle other){return py_cre_sub(self,other);}, nb::rv_policy::reference)
        .def("__rsub__", [](Literal* self, nb::handle other){return py_cre_rsub(self,other);}, nb::rv_policy::reference)
        .def("__mul__", [](Literal* self, nb::handle other){return py_cre_mul(self,other);}, nb::rv_policy::reference)
        .def("__rmul__", [](Literal* self, nb::handle other){return py_cre_rmul(self,other);}, nb::rv_policy::reference)
        .def("__truediv__", [](Literal* self, nb::handle other){return py_cre_truediv(self,other);}, nb::rv_policy::reference)
        .def("__rtruediv__", [](Literal* self, nb::handle other){return py_cre_rtruediv(self,other);}, nb::rv_policy::reference)
        .def("__floordiv__", [](Literal* self, nb::handle other){return py_cre_floordiv(self,other);}, nb::rv_policy::reference)
        .def("__rfloordiv__", [](Literal* self, nb::handle other){return py_cre_rfloordiv(self,other);}, nb::rv_policy::reference)
        .def("__mod__", [](Literal* self, nb::handle other){return py_cre_mod(self,other);}, nb::rv_policy::reference)
        .def("__rmod__", [](Literal* self, nb::handle other){return py_cre_rmod(self,other);}, nb::rv_policy::reference)
        .def("__pow__", [](Literal* self, nb::handle other){return py_cre_pow(self,other);}, nb::rv_policy::reference)
        .def("__rpow__", [](Literal* self, nb::handle other){return py_cre_rpow(self,other);}, nb::rv_policy::reference)
        .def("__neg__", [](Literal* self){return py_cre_neg(self);}, nb::rv_policy::reference)
        .def("__invert__", py_literal_invert, nb::rv_policy::reference)
        ;
        
    // Define Logic class
    nb::class_<Logic, CRE_Obj>(m, "Logic", nb::type_slots(cre_obj_slots))
        .def(nb::new_(&py_Logic_ctor), "kind"_a, "args"_a, nb::rv_policy::reference)
        .def("__str__", &Logic::to_string)
        .def("__repr__", &Logic::to_string)
        .def("to_string", &Logic::to_string)
        .def("basic_str", &Logic::basic_str)
        .def("standard_str", [](Logic* self, std::string_view indent) {
            return self->standard_str(indent);
        }, "indent"_a = "  ")
        .def_ro("kind", &Logic::kind)
        .def_ro("n_abs_vars", &Logic::n_abs_vars)
        .def("__len__", [](Logic* self) { return self->items.size(); })

        .def("antiunify", &py_Logic_antiunify, 
             "other"_a,
             "fixed_inds"_a = nb::none(),
             "return_score"_a = false,
             "fix_same_var"_a = false,
             nb::rv_policy::reference)
        ;
    
    // Expose AND and OR as module-level functions
    m.def("AND", &py_AND, "args"_a, nb::rv_policy::reference);
    m.def("OR", &py_OR, "args"_a, nb::rv_policy::reference);
}

