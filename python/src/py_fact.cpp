#include "../include/py_cre_core.h"
#include "../../include/fact.h"

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

    ref<Fact> fact = new_fact(type, items, items_len);
    // ref<Fact> fact_ref = ref<Fact>(fact);
    return fact;
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
    .def(nb::new_(&NewFact), nb::rv_policy::reference)
    // .def("get_refcount", &Fact::get_refcount)

    .def("__str__", &Fact::to_string, "verbosity"_a=2)
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
}
