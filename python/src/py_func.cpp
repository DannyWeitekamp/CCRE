
#include "../include/py_cre_core.h"
#include "../../include/item.h"
#include "../../include/types.h"
#include "../../include/func.h"
#include "../../include/builtin_funcs.h"


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

void* py_resolve_heads(void* dest, nb::object py_obj, const HeadInfo& hi){
    if (nb::isinstance<nb::bool_>(py_obj)) {
        return resolve_heads(dest, nb::cast<bool>(py_obj), hi);
    } else if (nb::isinstance<nb::int_>(py_obj)) {
        return resolve_heads(dest, nb::cast<int64_t>(py_obj), hi);
    } else if (nb::isinstance<nb::float_>(py_obj)) {
        return resolve_heads(dest, nb::cast<double>(py_obj), hi);
    } else if (nb::isinstance<nb::str>(py_obj)) {
        return resolve_heads(dest, nb::cast<std::string_view>(py_obj), hi);

        // InternStr intern_str = intern(nb::cast<std::string_view>(py_obj));
        // // cout << "Interned: " << intern_str << endl;
        // return Item(intern_str);

    } else if (nb::isinstance<Fact>(py_obj)) {
        return resolve_heads(dest, nb::cast<Fact*>(py_obj), hi);

        // return Item(nb::cast<Fact*>(py_obj));

    }else if (py_obj.is_none()){
        return resolve_heads(dest, Item(None), hi);

    } else {
        throw std::runtime_error("Item cannot be created from: " + nb::cast<std::string>(nb::str(py_obj)));
    }
}



nb::object py_Func_call(Func* func, nb::args args, nb::kwargs kwargs) {
    cout << "call() " << endl;

    if(kwargs.size() > 0){
        throw std::runtime_error("Not implemented: keyword args (kwargs) e.g. f(arg0=1, arg2=7).");
    }


    void** head_val_ptrs = (void**) alloca(sizeof(void**)*func->head_infos.size());
    uint8_t* stack = (uint8_t*) alloca(func->outer_stack_size);
    uint8_t* arg_head = stack;

    int64_t ret = 0;
    void* ret_ptr = (void*) (stack+func->outer_stack_size-func->return_type->byte_width);
    if(args.size() != func->n_args){
        func->throw_bad_n_args(args.size());
    }

    for(int i=0; i < args.size(); i++){
        auto arg = args[i];
        auto h_start = func->head_ranges[i].start;
        auto h_end = func->head_ranges[i].end;
    
        for(size_t head_ind=h_start; head_ind < h_end; ++head_ind){
            const HeadInfo& hi = func->head_infos[head_ind];
            void* head_val_ptr = py_resolve_heads(arg_head, arg, hi);

            if(head_val_ptr == nullptr){ [[unlikely]]
                func->throw_bad_arg(i, Item_from_py(arg), hi.base_type);
            }
            
            head_val_ptrs[head_ind] = head_val_ptr;
            arg_head += hi.base_type->byte_width;
        }
    }
    func->call_recursive_fc(func, ret_ptr, head_val_ptrs);

    if(func->has_outer_cleanup){
        for(size_t head_ind=0; head_ind < func->head_infos.size(); ++head_ind){
            const HeadInfo& hi = func->head_infos[head_ind];
            if(hi.head_type->dynamic_dtor != nullptr){
                // cout << ":::" << uint64_t(hi.head_type->dynamic_dtor) << endl;
                hi.head_type->dynamic_dtor(head_val_ptrs[head_ind]);
            }
        }   
    }

    return Item_to_py(func->ptr_to_item_func(ret_ptr));
}

ref<Func> py_Func_compose(Func* func, nb::args args) {
    cout << "compose() "  << endl;
    // Create a deep copy of the function
    FuncRef cf = func->copy_deep();
    
    // Check argument count
    if(args.size() != func->n_args){
        func->throw_bad_n_args(args.size());
    }
    
    // Convert each Python argument to Item and set it
    for(size_t i = 0; i < args.size(); ++i){
        Item arg_item = Item_from_py(args[i]);
        cf->set_arg(i, arg_item);
    }
    
    // Reinitialize the composed function
    cf->reinitialize();
    
    // Return the composed function
    return cf;
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

    cout << "__call__ " << has_var_or_func << endl;
    
    // If any argument is Var or Func, call compose
    if(has_var_or_func){
        return nb::cast(py_Func_compose(func, args));
    } else {
        // Otherwise, call the function directly
        return py_Func_call(func, args, kwargs);
    }
}

void init_func(nb::module_ & m){
	nb::class_<Func, CRE_Obj>(m, "Func", nb::type_slots(cre_obj_slots))
    // .def(nb::new_(&py_Func_ctor), nb::rv_policy::reference)


    // .def("get_refcount", &Fact::get_refcount)

    .def("__str__", &Func::to_string, "verbosity"_a=2)
    .def("__repr__", &Func::to_string, "verbosity"_a=2)
    .def("__call__", &py_Func__call__)
    .def("call", &py_Func_call)
    .def("compose", &py_Func_compose, nb::rv_policy::reference)

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
    m.attr("Equals") = ref<Func>(cre::Equals);
    m.attr("And") = ref<Func>(cre::And);
    m.attr("Or") = ref<Func>(cre::Or);
    m.attr("Xor") = ref<Func>(cre::Xor);
    m.attr("Not") = ref<Func>(cre::Not);
    m.attr("Add") = ref<Func>(cre::Add);
    m.attr("AddInts") = ref<Func>(cre::AddInts);
    m.attr("Subtract") = ref<Func>(cre::Subtract);
    m.attr("SubtractInts") = ref<Func>(cre::SubtractInts);
    m.attr("Multiply") = ref<Func>(cre::Multiply);
    m.attr("MultiplyInts") = ref<Func>(cre::MultiplyInts);
    m.attr("Divide") = ref<Func>(cre::Divide);
    m.attr("DivideInts") = ref<Func>(cre::DivideInts);
    m.attr("ModInts") = ref<Func>(cre::ModInts);
    m.attr("Pow") = ref<Func>(cre::Pow);
    m.attr("Neg") = ref<Func>(cre::Neg);
    m.attr("NegInt") = ref<Func>(cre::NegInt);
    m.attr("Concat") = ref<Func>(cre::Concat);
    m.attr("Slice") = ref<Func>(cre::Slice);
    m.attr("AddInts") = ref<Func>(cre::AddInts);
    // m.def("And", &cre::And, nb::rv_policy::reference);
    // m.def("Add", &cre::Add, nb::rv_policy::reference);
    // m.def("AddInts", &cre::AddInts, nb::rv_policy::reference);
    


}
