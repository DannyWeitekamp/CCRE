#include "../include/py_cre_core.h"
#include "../../include/var.h"



void init_logical(nb::module_ & m){
	nb::class_<Var>(m, "Var", nb::type_slots(cre_obj_slots))
    .def(nb::new_(
        [](CRE_Type* _type, std::string_view _alias){
            return new_var(_alias, _type);
        }),
        nb::rv_policy::reference)
    .def("__len__", &Var::size)
    ;
}
