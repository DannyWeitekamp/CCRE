#include "../include/py_cre_core.h"
#include "../../include/flattener.h"



void init_flattener(nb::module_ & m){
	nb::class_<Flattener>(m, "Flattener")
    // No inputs constructor (not sure why but this is necessary)
    .def(nb::new_(
        [](){
            return ref<Flattener>(new Flattener());
            }),
        nb::rv_policy::reference)

    // Constructor w/ inputs 
    .def(nb::new_([](FactSet* input, nb::handle flag_groups, bool subj_as_fact, int triple_order)->ref<Flattener>{
            std::vector<FlagGroup> flag_groups_vec;
            if(flag_groups.is_none()){
                flag_groups_vec = Flattener::default_flags;
            }else{
                flag_groups_vec = py_to_flag_groups(flag_groups);
            }
            // return ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
            return ref<Flattener>(new Flattener(input, flag_groups_vec, subj_as_fact, triple_order));
            }),
        "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "subj_as_fact"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO,              
        nb::rv_policy::reference)
    // .def(nb::new_(&py_new_flattener),
    //     "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "subj_as_fact"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO, 
    //     nb::rv_policy::reference)
    .def("apply", &Flattener::apply,
        // [](Flattener& self, FactSet* fs){
        //     return self.apply(fs);
        // },
        nb::rv_policy::reference)
    ;
}
