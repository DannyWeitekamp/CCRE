
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

nb::object py_fact_getitem(Fact* fact, int64_t index) {
    if(index < 0) index += fact->length;
    if(index < 0 || index >= fact->length){
        throw std::invalid_argument("Index ["+ std::to_string(index) +
            "] out of bounds for Fact with length=" + std::to_string(fact->length) + "."
        );
    }
    return Item_to_py(*fact->get(index));
}

nb::object py_fact_getattr(Fact* fact, std::string_view attr) {
    // if(index < 0) index += fact->length;
    // if(index < 0 || index >= fact->length){
    //     throw std::invalid_argument("Index ["+ std::to_string(index) +
    //         "] out of bounds for Fact with length=" + std::to_string(fact->length) + "."
    //     );
    // }
    return Item_to_py(*fact->get(attr));
}


class py_fact_iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = nb::object;
    using pointer           = nb::object*;
    using reference         = nb::object&;

    private:
        Item* current;
    public:
        explicit py_fact_iterator(Item* item_ptr) : current(item_ptr) {}
        nb::object operator*() const { return Item_to_py(*current); }
        // pointer operator->() const { return &Item_to_py(*current); }
        py_fact_iterator& operator++() { ++current; return *this; }
        bool operator!=(const py_fact_iterator& other) const { return current != other.current; }
        bool operator==(const py_fact_iterator& other) const { return current == other.current; }
};

py_fact_iterator py_fact_iter_begin(Fact* self){
    uint8_t* data = (uint8_t*) self;
    return py_fact_iterator((Item*) (data + sizeof(Fact)));
}

py_fact_iterator py_fact_iter_end(Fact* self){
    uint8_t* data = (uint8_t*) self;
    return py_fact_iterator((Item*) (data + sizeof(Fact) + self->length*sizeof(Item)));
}


void init_fact(nb::module_ & m){
	nb::class_<Fact, CRE_Obj>(m, "Fact", nb::type_slots(cre_obj_slots))
    .def(nb::new_(&py_Fact_ctor), nb::rv_policy::reference)
    // .def("get_refcount", &Fact::get_refcount)

    .def("__str__", &Fact::to_string, "verbosity"_a=2)
    .def("__repr__", &Fact::to_string, "verbosity"_a=2)
    .def("__len__", &Fact::size)
    
    .def("__getitem__", &py_fact_getitem)

    .def("__getitem__", &py_fact_getattr)

    .def("__getattr__", &py_fact_getattr)

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
    ;

    m.def("NewFact", &NewFact, nb::rv_policy::reference);
    m.def("iFact", &py_iFact_ctor, nb::rv_policy::reference);
}
