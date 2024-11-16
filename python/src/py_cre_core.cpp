#include "../include/py_cre_core.h"
#include "../../external/nanobind/src/nb_internals.h"

// ---------------------------------------------------------------
// : Deallocator For decref()'ing on Garbage Collect  

// Forward declare nb::detail::inst_dealloc()
// NAMESPACE_BEGIN(NB_NAMESPACE)
// NAMESPACE_BEGIN(detail)
// static void inst_dealloc(PyObject *self);
// NAMESPACE_END(detail)
// NAMESPACE_END(NB_NAMESPACE)

typedef void (*dealloc_ty)(PyObject *self);
dealloc_ty nb_inst_dealloc = nullptr;

static void cre_obj_dealloc(PyObject *self){
    auto self_handle = nb::handle(self);
    CRE_Obj* cpp_self = (CRE_Obj*) nb::inst_ptr<CRE_Obj>(self_handle);


    
    // inst_dealloc(self);
    // if(nb_inst_dealloc){
    
    // }
    // When the Python proxy object is collected remove 
    //   the C++ object's reference to it so it isn't reused
    cpp_self->proxy_obj = nullptr;
    if(((nb::detail::nb_inst*) self)->unused == 17){
    	// nb::print("~Borrowed Died~: " + nb::cast<std::string>(nb::str(self_handle)) + "refs=" + std::to_string(cpp_self->get_refcount()));
    	// cout << "~Borrowed Died~:" << cpp_self->get_refcount() << endl;    
    	// cout << "~Borrowed Died~: " << nb::cast<std::string>(nb::str(self_handle)) << "refs=" << std::to_string(cpp_self->get_refcount()) << endl;    
    	cpp_self->dec_ref();	
    }else{
    	// cout << "~Ptr Died~:" << cpp_self->get_refcount() << endl;    
    }

    (*nb_inst_dealloc)(self);
}

PyType_Slot cre_obj_slots[] = {
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

    } else if (nb::isinstance<Fact>(py_obj)) {
    	return Item(nb::cast<Fact*>(py_obj));

    }else if (py_obj.is_none()){
        return Item();
    } else {
        throw std::runtime_error("Item cannot be created from: " + nb::cast<std::string>(nb::str(py_obj)));
        return Item();
    }
}

nb::object Item_to_py(Item item){
	uint16_t t_id = item.t_id;
	switch(t_id) {
        case T_ID_NULL:
           	return nb::none();
        case T_ID_BOOL:
            return nb::cast(item.as_bool());
        case T_ID_INT:
        	return nb::cast(item.as_int());
        case T_ID_FLOAT:
        	return nb::cast(item.as_float());
        case T_ID_STR:
        	return nb::cast(item.as_string());
        case T_ID_FACT:
            return nb::cast(ref<Fact>(std::bit_cast<Fact*>(item.val)));
        default:
        	throw std::runtime_error("Conversion Not Implemented t_id="+std::to_string(t_id));    
    }
}

// ------------------------------------------------------------------
// : Type Conversion

std::string Type_name_from_py(nb::handle py_obj){
    // nb:print("Type name from py: ", py_obj);
    std::string type_name = "";
    if(nb::isinstance<nb::str>(py_obj)){
        type_name = nb::cast<std::string>(py_obj);
    }else if(nb::isinstance<CRE_Type>(py_obj)){
    	type_name = nb::cast<CRE_Type*>(py_obj)->name;
    	// cout << "IS CRE_TYPE: " << type_name << endl;
    }else if(py_obj.is_type() && !py_obj.is_none()){
        type_name = nb::cast<std::string>(nb::getattr(py_obj, "__name__"));
    }else{
        throw std::runtime_error("Expecting string or type object to resolve type name, but got: \"" + nb::cast<std::string>(nb::str(py_obj)) + "\"");
    }
    return type_name;
}

FactType* FactType_from_py(nb::handle py_obj){
    std::string type_name = Type_name_from_py(py_obj);
    return current_context->get_fact_type(type_name); // throws if not found
}

CRE_Type* Type_from_py(nb::handle py_obj){
    std::string type_name = Type_name_from_py(py_obj);
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

        // CRE_Type* mbr_type = nullptr;
        // if(nb::isinstance<nb::str>(type_handle)){
            // auto type_name = Type_from_py(type_handle);//::cast<std::string>(type_handle);
        // mbr_type = Type_from_py(type_handle);//current_context->get_type(type_name);
        std::string mbr_type_name = Type_name_from_py(type_handle);
        // if(mbr_type == nullptr){
        //     throw std::runtime_error("Fact type not found.");
        // }
        // }

        HashMap<std::string, Item> flags = {};
        if(py_flags){
            nb::dict _py_flags = *py_flags;
            for (auto [py_flag_name, py_flag_val] : _py_flags) {
                flags[nb::cast<std::string>(py_flag_name)] = Item_from_py(py_flag_val);
            }
        }

        std::string attr_name = nb::cast<std::string>(py_attr_name);
        MemberSpec member_spec = MemberSpec(attr_name, mbr_type_name, flags);//py_flags ? 
        // MemberSpec(attr_name, mbr_type_name, flags);// :
            // MemberSpec(attr_name, mbr_type_name, {});
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

	// -- CRE_Obj --
	nb::class_<CRE_Obj>(m, "CRE_Obj", nb::type_slots(cre_obj_slots))
    .def("get_refcount", &CRE_Obj::get_refcount)
    ;

    // -- CRE_Type --
    nb::class_<CRE_Type>(m, "CRE_Type", nb::type_slots(cre_obj_slots))
    ;

    // -- FactType ---
    nb::class_<FactType, CRE_Type>(m, "Fact_Type", nb::type_slots(cre_obj_slots))
    .def("__call__", [](FactType& self, nb::args args, nb::kwargs kwargs){
        return _py_new_fact(&self, args.size(), args.begin(), args.end(), kwargs);
    }, nb::rv_policy::reference)
    .def("__len__", [](FactType& self){
        return self.members.size();
    })
    ;

    // -- DefferedType ---
    nb::class_<DefferedType, CRE_Type>(m, "DefferedType", nb::type_slots(cre_obj_slots))
    .def(nb::init<std::string_view>(), "name"_a)
    ;

    // -- CRE_Context --
    nb::class_<CRE_Context>(m, "CRE_Context", nb::type_slots(cre_obj_slots))
    .def("get_type", &CRE_Context::get_type, nb::rv_policy::reference)
    .def("get_fact_type", &CRE_Context::get_fact_type, nb::rv_policy::reference)
    .def("__str__", &CRE_Context::to_string)
    .def(nb::new_([](){
    	return current_context;
    }), nb::rv_policy::reference)
    .def(nb::new_([](std::string_view name){
    	return get_context(name);
    }), nb::rv_policy::reference)

    // .def_static("__call__", [](){
    // 	return current_context;
    // })
    // .def_static("__call__", [](std::string_view name){
    // 	return get_context(name);
    // })
    ;


    nb::class_<Item>(m, "Item")
    ;


    m.def("define_fact", &py_define_fact, nb::rv_policy::reference);
}

// -----------------------------------------------------------------
// :  _py_new_fact:  Helper function for making fact from args, kwargs

ref<Fact> _py_new_fact(FactType* type, 
                int n_pos_args,
                nb::detail::fast_iterator args_start,
                nb::detail::fast_iterator args_end,
                nb::kwargs kwargs,
                bool immutable){
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

    ref<Fact> fact = new_fact(type, items, items_len);
    fact->immutable = immutable;
    // ref<Fact> fact_ref = ref<Fact>(fact);
    return fact;
}
