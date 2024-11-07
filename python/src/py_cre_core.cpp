#include "../include/py_cre_core.h"

// ---------------------------------------------------------------
// : Deallocator For decref()'ing on Garbage Collect  

typedef void (*dealloc_ty)(PyObject *self);
dealloc_ty nb_inst_dealloc = nullptr;

static void cre_obj_dealloc(PyObject *self){
    auto self_handle = nb::handle(self);
    CRE_Obj* cpp_self = (CRE_Obj*) nb::inst_ptr<CRE_Obj>(self_handle);


    // nb::print("DIED: ", self_handle);
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
// : Item Conversion

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

// ------------------------------------------------------------------
// : Type Conversion

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

// ------------------------------------------------------------------
// : Flag Group Conversion

FlagGroup dict_to_flag_group(nb::dict py_dict){
    FlagGroup flag_group = {};
    for (auto [key, val] : py_dict) {
        flag_group.assign_flag(nb::cast<std::string>(key), Item_from_py(val));
    }
    return flag_group;
}

std::vector<FlagGroup> py_to_flag_groups(nb::handle py_obj){
    std::vector<FlagGroup> flags = {};
    if(nb::isinstance<nb::dict>(py_obj)){
        nb::dict py_dict = nb::cast<nb::dict>(py_obj);
        flags = {dict_to_flag_group(py_dict)};    
    }else if(nb::isinstance<nb::list>(py_obj)){
        nb::list py_list = nb::cast<nb::list>(py_obj);
        flags.reserve(py_list.size());
        for(auto py_flag_group : py_list){
            if(nb::isinstance<nb::dict>(py_flag_group)){
                flags.push_back(dict_to_flag_group(nb::cast<nb::dict>(py_flag_group)));
            }else{
                throw std::runtime_error("Expecting a dict in flag group list, but got: " + nb::cast<std::string>(nb::str(py_flag_group)));
            }
        }
    }else{
        throw std::runtime_error("Expecting a list of dicts or single dict for flag groups, but got: " + nb::cast<std::string>(nb::str(py_obj)));
    }
    return flags;
}

// ------------------------------------------------------------------
// : define_fact()

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


// ------------------------------------------------------------------
// : Initialize CRE_Obj, CRE_Type, Fact_Type, Item

void init_core(nb::module_& m) {
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

    nb::class_<CRE_Type>(m, "CRE_Type")
    ;

    nb::class_<FactType>(m, "Fact_Type")
    // Constructor MyFactType(...)
    .def("__call__", [](FactType& self, nb::args args, nb::kwargs kwargs){
        return _py_new_fact(&self, args.size(), args.begin(), args.end(), kwargs);
    })
    ;

    nb::class_<Item>(m, "Item")
    ;


    m.def("define_fact", &py_define_fact, nb::rv_policy::reference);
}
