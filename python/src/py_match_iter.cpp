#include "../include/py_cre_core.h"
#include "../include/shared_logic_utils.h"
#include "../../include/corgi.h"


ref<Mapping> py_MatchIter__next__(MatchIter* self) {
    ref<Mapping> match = self->curr_match->copy();
    self->operator++();
    return match;
}

class py_mapping_iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = nb::object;
    using pointer           = nb::object*;
    using reference         = nb::object&;

    private:
        MappingIter iter;
    public:
        explicit py_mapping_iterator(MappingIter iter) : 
            iter(iter) {};
        nb::object operator*() const { return Item_to_py(*iter); }
        // pointer operator->() const { return &Item_to_py(*current); }
        py_mapping_iterator& operator++() { ++iter; return *this; }
        bool operator!=(const py_mapping_iterator& other) const { return iter != other.iter; }
        bool operator==(const py_mapping_iterator& other) const { return iter == other.iter; }
};


void init_mapping(nb::module_ & m) {
    nb::class_<Mapping, CRE_Obj>(m, "Mapping", nb::type_slots(cre_obj_slots))

    .def("__iter__",  [](Mapping* mapping) {
        return nb::make_iterator(nb::type<Fact>(), "iterator",
                                 py_mapping_iterator(mapping->begin()),
                                 py_mapping_iterator(mapping->end()));
     })
    ;
}

void init_match_iter(nb::module_ & m) {
    nb::class_<MatchIter, CRE_Obj>(m, "MatchIter", nb::type_slots(cre_obj_slots))
        .def("__next__", &py_MatchIter__next__,nb::rv_policy::reference)
        .def("__iter__", [](MatchIter* self){return self;}, nb::rv_policy::reference)
    ;
}

