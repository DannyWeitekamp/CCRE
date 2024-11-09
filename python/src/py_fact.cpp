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

    Fact* fact = new_fact(type, items, items_len);
    ref<Fact> fact_ref = ref<Fact>(fact);
    return fact_ref;
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


void init_fact(nb::module_ & m){
	nb::class_<Fact>(m, "Fact", nb::type_slots(cre_obj_slots))
    .def(nb::new_(&NewFact), nb::rv_policy::reference)
    .def("__str__", &Fact::to_string, "verbosity"_a=2)
    .def("__len__", &Fact::size)
    .def("get_refcount", &Fact::get_refcount)
    ;

    m.def("NewFact", &NewFact, nb::rv_policy::reference);
}
