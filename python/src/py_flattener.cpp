#include "../include/py_cre_core.h"
#include "../../include/flattener.h"



// ref<Flattener> py_Flattener_ctor(FactSet* input, nb::handle flag_groups, bool use_vars, bool add_exist_stubs, int triple_order){

// }

void init_flattener(nb::module_ & m){
	nb::class_<Flattener>(m, "Flattener", nb::type_slots(cre_obj_slots))
    
    // .def(nb::new_(
    //     [](){return ref<Flattener>(new Flattener());}),
    //     nb::rv_policy::reference
    // )
    
    
    // Constructor w/ & w/o inputs 
    // Always must start with no inputs constructor, this counts because all defaults are defined
    .def(nb::new_(
        [](FactSet* input, nb::handle flag_groups, bool use_vars, bool add_exist_stubs, int triple_order)->ref<Flattener>{
            std::vector<FlagGroup> flag_groups_vec;
            if(flag_groups.is_none()){
                flag_groups_vec = Flattener::default_flags;
            }else{
                flag_groups_vec = py_to_flag_groups(flag_groups);
            }
            // return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, triple_order));
            return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, add_exist_stubs, triple_order));
            }),

        
        "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "use_vars"_a=false, "add_exist_stubs"_a=false, "triple_order"_a=TRIPLE_ORDER_SVO,              
        nb::rv_policy::reference)
    // .def(nb::new_(&py_new_flattener),
    //     "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "use_vars"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO, 
    //     nb::rv_policy::reference)
    .def("apply", &Flattener::apply,
        // [](Flattener& self, FactSet* fs){
        //     return self.apply(fs);
        // },
        nb::rv_policy::reference)
    ;
}



// #include "../include/py_cre_core.h"
// #include "../../include/flattener.h"



// ref<Flattener> py_Flattener_ctor(FactSet* input, nb::handle flag_groups, bool use_vars, bool add_exist_stubs, int triple_order){
//     std::vector<FlagGroup> flag_groups_vec;
//     if(flag_groups.is_none()){
//         flag_groups_vec = Flattener::default_flags;
//     }else{
//         flag_groups_vec = py_to_flag_groups(flag_groups);
//     }
//     // return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, triple_order));
//     return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, add_exist_stubs, triple_order));
// };

// void init_flattener(nb::module_ & m){
//     nb::class_<Flattener>(m, "Flattener", nb::type_slots(cre_obj_slots))
//     // No inputs constructor (not sure why but this is necessary)
//     .def("__new__", 
//         [](){return ref<Flattener>(new Flattener());},
//         nb::rv_policy::reference
//     )

//     // Constructor w/ inputs 
//     .def("__new__", &py_Flattener_ctor,
//         "input"_a=nb::none(), "flag_groups"_a=nb::none(), "use_vars"_a=false, "add_exist_stubs"_a=false, "triple_order"_a=TRIPLE_ORDER_SVO,
//         nb::rv_policy::reference
//     )

//     .def("apply", &Flattener::apply,
//         // [](Flattener& self, FactSet* fs){
//         //     return self.apply(fs);
//         // },
//         nb::rv_policy::reference)
//         // [](FactSet* input, nb::handle flag_groups, bool use_vars, bool add_exist_stubs, int triple_order)->ref<Flattener>{
//         //     std::vector<FlagGroup> flag_groups_vec;
//         //     if(flag_groups.is_none()){
//         //         flag_groups_vec = Flattener::default_flags;
//         //     }else{
//         //         flag_groups_vec = py_to_flag_groups(flag_groups);
//         //     }
//         //     // return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, triple_order));
//         //     return ref<Flattener>(new Flattener(input, flag_groups_vec, use_vars, add_exist_stubs, triple_order));
//         //     }),
        
        
//     // .def(nb::new_(&py_new_flattener),
//     //     "input"_a.none()=nb::none(), "flag_groups"_a.none()=nb::none(), "use_vars"_a=false, "triple_order"_a=TRIPLE_ORDER_SPO, 
//     //     nb::rv_policy::reference)
    
//     ;
// }
