#include "../include/py_cre_core.h"
#include "../../include/item.h"
#include "../../include/types.h"
#include "../../include/func.h"
#include "../../include/logic.h"
#include "../../include/builtin_funcs.h"  
#include "../../include/literal.h"  
#include "../include/nanobind/nanobind.h"

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

Item py_Func_call_to_item(Func* func, nb::args args, nb::kwargs kwargs) {
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

    return func->ptr_to_item_func(ret_ptr);
}

ref<Func> py_Func_compose(Func* func, nb::args args) {
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
