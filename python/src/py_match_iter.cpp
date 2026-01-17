#include "../include/py_cre_core.h"
#include "../include/shared_logic_utils.h"
#include "../../include/corgi.h"


ref<Mapping> py_MatchIter__next__(MatchIter* self) {
    ref<Mapping> match = self->curr_match->copy();
    self->operator++();
    return match;
}

void init_match_iter(nb::module_ & m) {
    nb::class_<MatchIter, CRE_Obj>(m, "MatchIter", nb::type_slots(cre_obj_slots))
        .def("__next__", &py_MatchIter__next__,nb::rv_policy::reference)
        .def("__iter__", [](MatchIter* self){return self;}, nb::rv_policy::reference)
    ;
}