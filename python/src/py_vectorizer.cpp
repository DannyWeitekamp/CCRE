#include "../include/py_cre_core.h"
#include "../../include/vectorizer.h"


using uint_arr = nb::ndarray<uint64_t, nb::numpy, nb::ndim<1>, nb::c_contig>;
using double_arr = nb::ndarray<double, nb::numpy, nb::ndim<1>, nb::c_contig>;

// std::tuple<uint_arr, double_arr> 
nb::tuple py_Vectorizer_apply(nb::handle self, FactSet* input){
    Vectorizer* cpp_self = (Vectorizer*) nb::inst_ptr<Vectorizer>(self);
    auto [nom, cont] = cpp_self->apply(input);

    auto nom_arr = uint_arr
                    (nom->data(), {nom->size()}, self);
    auto cont_arr = double_arr
                    (cont->data(), {cont->size()}, self);
    return nb::make_tuple(nom_arr, cont_arr);
}


void init_vectorizer(nb::module_ & m){
	nb::class_<Vectorizer>(m, "Vectorizer", nb::type_slots(cre_obj_slots))
    .def(nb::new_(
        [](){
            return ref<Vectorizer>(new Vectorizer());
            }),
        nb::rv_policy::reference)

    .def(nb::new_([](uint64_t max_heads, bool one_hot_nominals, bool encode_missing)->ref<Vectorizer>{
            return ref<Vectorizer>(new Vectorizer(max_heads, one_hot_nominals, encode_missing));
        }), "max_heads"_a=0, "one_hot_nominals"_a=true, "encode_missing"_a=false,
        nb::rv_policy::reference)
    // .def(nb::init<uint64_t, bool, bool>(), "max_heads"_a=0, "one_hot_nominal"_a=true, "encode_missings"_a=false,
    //     nb::rv_policy::reference)
    .def("apply", &py_Vectorizer_apply)
        // nb::rv_policy::reference)
    // .def("invert", nb::overload_cast<size_t,size_t>(&Vectorizer::invert))
    .def("invert", [](Vectorizer& self, size_t slot, size_t value) {
    	return ref<Fact>(self.invert(slot, value));
    }, nb::rv_policy::reference)
    .def("invert", [](Vectorizer& self, size_t slot, double value) {
    	return ref<Fact>(self.invert(slot, value));
    }, nb::rv_policy::reference)
    ;
}

