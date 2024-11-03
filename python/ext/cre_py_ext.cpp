#include <iostream>
#include <string>
#include <atomic>
#include <memory> 

// Note: these need to included in this order
#include <nanobind/nanobind.h>
#include "../../include/cre_obj.h"
#include "../../include/ref.h"
// 

#include "../../include/context.h"
#include "../../include/types.h"
#include "../../include/intern.h"
#include "../../include/fact.h"
#include "../../include/factset.h"

#include <nanobind/stl/string.h>
#include <nanobind/stl/string_view.h>

namespace nb = nanobind;
using namespace nb::literals;
using std::cout;
using std::endl;


// ---------------------------------------------------------------
// : Deallocator For decref()'ing on Garbage Collect  


typedef void (*dealloc_ty)(PyObject *self);
dealloc_ty nb_inst_dealloc = nullptr;

static void cre_obj_dealloc(PyObject *self){
    auto self_handle = nb::handle(self);
    CRE_Obj* cpp_self = (CRE_Obj*) nb::inst_ptr<CRE_Obj>(self_handle);

    // cout << "~Before Die~:" << cpp_self->get_refcount() << endl;    

    if(nb_inst_dealloc){
        (*nb_inst_dealloc)(self);
    }
    // When the Python proxy object is collected remove 
    //   the C++ object's reference to it so it isn't reused
    cpp_self->proxy_obj = nullptr;
    CRE_decref(cpp_self);//->dec_ref();
}

// PyObject *myclass_tp_add(PyObject *a, PyObject *b) {
//     return PyNumber_Multiply(a, b);
// }

PyType_Slot slots[] = {
    { Py_tp_dealloc, (void *) cre_obj_dealloc },
    { 0, nullptr }
};

class CREDummy : public CRE_Obj{
    int a;
};

// ------------------------------------------------------------------
// : Nanobind Module



Item Item_from_py(nb::handle py_obj){
    if (nb::isinstance<nb::bool_>(py_obj)) {
        return Item(nb::cast<bool>(py_obj));
    } else if (nb::isinstance<nb::int_>(py_obj)) {
        return Item(nb::cast<int>(py_obj));
    } else if (nb::isinstance<nb::float_>(py_obj)) {
        return Item(nb::cast<double>(py_obj));
    } else if (nb::isinstance<nb::str>(py_obj)) {
        InternStr intern_str = intern(nb::cast<std::string_view>(py_obj));
        // cout << "Interned: " << intern_str << endl;
        return Item(intern_str);
    
    } else {
        throw std::runtime_error("Item cannot be created from: " + nb::cast<std::string>(nb::str(py_obj)));
    }
}

std::string_view Type_name_from_py(nb::handle py_obj){
    // nb:print("Type name from py: ", py_obj);
    std::string_view type_name = "";
    if(nb::isinstance<nb::str>(py_obj)){
        type_name = nb::cast<std::string_view>(py_obj);
    }else if(py_obj.is_type() && !py_obj.is_none()){
        type_name = nb::cast<std::string_view>(nb::getattr(py_obj, "__name__"));
    }else{
        throw std::runtime_error("Expecting string or type object to resolve type name, but got: \"" + nb::cast<std::string>(nb::str(py_obj)) + "\"");
    }
    return type_name;
}

FactType* FactType_from_py(nb::handle py_obj){
    std::string_view type_name = Type_name_from_py(py_obj);
    return current_context->get_fact_type(type_name); // throws if not found
}

CRE_Type* Type_from_py(nb::handle py_obj){
    std::string_view type_name = Type_name_from_py(py_obj);
    return current_context->get_type(type_name); // throws if not found
}

FactType* py_define_fact(nb::str py_name, nb::dict member_infos){
    // cout << current_context->to_string() << endl;

    FactType* type = nullptr;

    std::vector<MemberSpec> members = {};
    for (auto [py_attr_name, val] : member_infos) {
        nb::handle type_handle = val;
        std::optional<nb::dict> py_flags;
        
        if(nb::isinstance<nb::tuple>(val)){
            auto tuple = nb::cast<nb::tuple>(val);
            if(tuple.size() >= 1){
                type_handle = tuple[0];
            }
            if(tuple.size() >= 2){
                py_flags = nb::cast<nb::dict>(tuple[1]);
            }
        }

        CRE_Type* mbr_type = nullptr;
        // if(nb::isinstance<nb::str>(type_handle)){
            // auto type_name = Type_from_py(type_handle);//::cast<std::string>(type_handle);
        mbr_type = Type_from_py(type_handle);//current_context->get_type(type_name);
        if(mbr_type == nullptr){
            throw std::runtime_error("Fact type not found.");
        }
        // }

        HashMap<std::string, Item> flags = {};
        if(py_flags){
            nb::dict _py_flags = *py_flags;
            for (auto [py_flag_name, py_flag_val] : _py_flags) {
                flags[nb::cast<std::string>(py_flag_name)] = Item_from_py(py_flag_val);
            }
        }

        std::string attr_name = nb::cast<std::string>(py_attr_name);
        MemberSpec member_spec = py_flags ? 
            MemberSpec(attr_name, mbr_type, flags) :
            MemberSpec(attr_name, mbr_type);
        members.push_back(member_spec);
    }
    std::string name = nb::cast<std::string>(py_name);
    return define_fact(name, members);
}


ref<Fact> _py_new_fact(FactType* type, 
                int n_pos_args,
                nb::detail::fast_iterator args_start,
                nb::detail::fast_iterator args_end,
                nb::kwargs kwargs){
    if(type == nullptr && kwargs.size() > 0){
        throw std::invalid_argument("Keyword argument used in untyped Fact initialization.");
    }

    int max_args = n_pos_args + kwargs.size();
    int items_len = n_pos_args;
    Item* items = (Item*) alloca(sizeof(Item) * max_args);
    std::fill(items, items+max_args, Item());

    auto it = args_start;
    for (int i=0; it != args_end; ++it, i++) {
        items[i] = Item_from_py(*it);
        // nb::print(nb::str("Positional: {}").format(*it));
    }

    for (auto [key, val] : kwargs) {
        std::string_view key_view = nb::cast<std::string_view>(key);
        int mbr_ind = type->get_attr_index(key_view);
        if(mbr_ind == -1){
            throw std::invalid_argument("FactType: \"" + type->name + "\" has no member: \"" + std::string(key_view) + "\"");
        }
        if(mbr_ind < n_pos_args){
            throw std::invalid_argument("Keyword argument \"" + std::string(key_view) + "\" overwrites positional argument " + std::to_string(mbr_ind));
        }
        items[mbr_ind] = Item_from_py(val);
        items_len = std::max(items_len, mbr_ind+1);
    }

    Fact* fact = new_fact(type, items, items_len);
    ref<Fact> fact_ref = ref<Fact>(fact);
    return fact_ref;;
}

ref<Fact> NewFact(nb::args args, nb::kwargs kwargs) {
    FactType* fact_type = nullptr;
    auto it = args.begin();
    if(args.size() >= 1){
        CRE_Context* context = current_context;
        auto arg0 = *it;
        
        if(!arg0.is_none()){ 
            fact_type = FactType_from_py(arg0);
        }
        // nb::print(nb::str("args[0]: {}").format(arg0));
        it++;
    }
    return _py_new_fact(fact_type, args.size()-1, it, args.end(), kwargs);  
}





NB_MODULE(cre, m) {
    // Define a dummy Type and use it to set the address of nb_inst_dealloc
    nb::class_<CREDummy>(m, "CREDummy");
    auto dum_ty_handle = nb::type<CREDummy>();
    if (dum_ty_handle.is_valid()) {
        PyTypeObject* obj_ty = (PyTypeObject*) dum_ty_handle.ptr();
        nb_inst_dealloc = obj_ty->tp_dealloc;
    }

	// A very minimal wrapper for a CRE_obj Python side 
	nb::class_<CRE_Obj>(m, "CRE_Obj")
    .def("get_refcount", &CRE_Obj::get_refcount)
    ;

    nb::class_<CRE_Type>(m, "CRE_Type");

    nb::class_<FactType>(m, "Fact_Type")
    // Constructor MyFactType(...)
    .def("__call__", [](FactType& self, nb::args args, nb::kwargs kwargs){
        return _py_new_fact(&self, args.size(), args.begin(), args.end(), kwargs);
    })
    // .def(nb::new_(&NewFact), nb::rv_policy::reference)
    ;

    // Fact
    nb::class_<Item>(m, "Item");

    // Fact
    nb::class_<Fact>(m, "Fact")
    .def(nb::new_(&NewFact), nb::rv_policy::reference)
    .def("__str__", &Fact::to_string)
    .def("__len__", &Fact::size)
    .def("get_refcount", &Fact::get_refcount)
    ;

    m.def("NewFact", &NewFact, nb::rv_policy::reference);
    m.def("define_fact", &py_define_fact, nb::rv_policy::reference);


}
