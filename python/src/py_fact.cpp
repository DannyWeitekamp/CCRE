
#include "../include/py_cre_core.h"
#include "../../include/fact.h"


ref<Fact> py_Fact_ctor(nb::args args) {
    nb::kwargs kwargs; // stub 
    return _py_new_fact(nullptr, args.size(), args.begin(), args.end(), kwargs, false);
}

ref<Fact> py_iFact_ctor(nb::args args) {
    nb::kwargs kwargs; // stub 
    return _py_new_fact(nullptr, args.size(), args.begin(), args.end(), kwargs, true);
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
    return _py_new_fact(fact_type, args.size()-1, it, args.end(), kwargs, false);  
}


ref<Fact> py_Tuple_ctor(nb::args args) {
    nb::kwargs kwargs; // stub 
    return _py_new_fact(nullptr, args.size(), args.begin(), args.end(), kwargs, true);
}
nb::object py_fact_getitem_index(Fact* fact, int64_t index) {
    if(index < 0) index += fact->length;
    if(index < 0 || index >= fact->length){
        throw nb::index_error(("Index ["+ std::to_string(index) +
            "] out of bounds for Fact with length=" + std::to_string(fact->length) + ".").c_str()
        );
    }
    return Item_to_py(fact->get(index));
}

// bool py_fact_hasattr(Fact* fact, std::string_view attr) {
//     cout << "HAS ATTR" << endl;
//     try {
//         fact->get(attr);
//     } catch (const std::exception& e) {
//         return false;
//     } 
//     return true;
// }

nb::object py_fact_getitem_str(Fact* fact, std::string_view attr) {
    try {
        return Item_to_py(fact->get(attr));
    } catch (const std::exception& ex) {
        throw nb::key_error(ex.what());
    }     
}

nb::object py_fact_getattr(Fact* fact, std::string_view attr) {
    try {
        return Item_to_py(fact->get(attr));
    } catch (const std::exception& ex) {
        throw nb::attribute_error(ex.what());
    }     
}



void py_fact_setitem(Fact* fact, int64_t index, nb::handle py_val) {
    if(index < 0) index += fact->length;
    if(index < 0 || index >= fact->length){
        throw std::invalid_argument("Index ["+ std::to_string(index) +
            "] out of bounds for Fact with length=" + std::to_string(fact->length) + "."
        );
    }
    fact->set(index, Item_from_py(py_val));
}


void py_fact_setattr(Fact* fact, std::string_view attr, nb::handle py_val) {
    fact->set(attr, Item_from_py(py_val));
}


class py_fact_iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = nb::object;
    using pointer           = nb::object*;
    using reference         = nb::object&;

    private:
        Fact* fact;
        size_t ind;
    public:
        explicit py_fact_iterator(Fact* fact, size_t ind=0) : 
            fact(fact), ind(ind) 
        {};
        nb::object operator*() const { return Item_to_py(fact->get(ind)); }
        // pointer operator->() const { return &Item_to_py(*current); }
        py_fact_iterator& operator++() { ++ind; return *this; }
        bool operator!=(const py_fact_iterator& other) const { return ind != other.ind; }
        bool operator==(const py_fact_iterator& other) const { return ind == other.ind; }
};

py_fact_iterator py_fact_iter_begin(Fact* self){
    uint8_t* data = (uint8_t*) self;
    // return py_fact_iterator((Member*) (data + sizeof(Fact)));
    return py_fact_iterator(self, 0);
}

py_fact_iterator py_fact_iter_end(Fact* self){
    uint8_t* data = (uint8_t*) self;
    return py_fact_iterator(self, self->length);
    // return py_fact_iterator((Member*) (data + sizeof(Fact) + self->length*sizeof(Member)));
}

void init_fact(nb::module_ & m){
	nb::class_<Fact, CRE_Obj>(m, "Fact", nb::type_slots(cre_obj_slots))
    .def(nb::new_(&py_Fact_ctor), nb::rv_policy::reference)
    // .def("get_refcount", &Fact::get_refcount)

    .def("__str__", &Fact::to_string, "verbosity"_a=2)
    .def("__repr__", &Fact::to_string, "verbosity"_a=2)
    .def("__len__", &Fact::size)
    
    .def("__getitem__", &py_fact_getitem_index)
    .def("__getitem__", &py_fact_getitem_str)
    .def("__getattr__", &py_fact_getattr)

    .def("__setitem__", &py_fact_setitem)
    .def("__setattr__", &py_fact_setattr)

    .def("isa", [](Fact* fact, nb::handle type){
        return fact->isa(Type_from_py(type));
    }, "type"_a)
    .def("issubclass", [](Fact* fact, nb::handle type){
        return fact->issubclass(Type_from_py(type));
    }, "type"_a)
    .def("__iter__",  [](Fact* fact) {
            return nb::make_iterator(nb::type<Fact>(), "iterator",
                                     py_fact_iter_begin(fact),
                                     py_fact_iter_end(fact)
                                     );
        }, nb::keep_alive<0, 1>()//, //nb::rv_policy::reference
    )
    .def("iter_items",  [](Fact& fact) {
            return nb::make_iterator(nb::type<Fact>(), "iterator",
                                     fact.begin(), fact.end());
        }, nb::keep_alive<0, 1>()//, //nb::rv_policy::reference
    )
    .def_ro("_fact_type", &Fact::type)
    ;

    m.def("NewFact", &NewFact, nb::rv_policy::reference);
    m.def("iFact", &py_iFact_ctor, nb::rv_policy::reference);
    m.def("Tuple", &py_Tuple_ctor, nb::rv_policy::reference);
}
