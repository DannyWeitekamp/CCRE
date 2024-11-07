#include "../include/py_cre_core.h"
#include "../../include/vectorizer.h"


using uint_arr = nb::ndarray<uint64_t, nb::numpy, nb::ndim<1>>;
using double_arr = nb::ndarray<double, nb::numpy, nb::ndim<1>>;

// std::tuple<uint_arr, double_arr> 
nb::tuple
    py_Vectorizer_apply(Vectorizer& self, FactSet* input){
    auto [nom, cont] = self.apply(input);

    auto nom_arr = uint_arr
                    (nom.data(), {nom.size()});
    auto cont_arr = double_arr
                    (cont.data(), {cont.size()});
    return nb::make_tuple(nom_arr, cont_arr);
}


void init_vectorizer(nb::module_ & m){
	nb::class_<Vectorizer>(m, "Vectorizer")
    .def(nb::new_(
        [](){
            return ref<Vectorizer>(new Vectorizer());
            }),
        nb::rv_policy::reference)

    .def(nb::new_([](uint64_t max_heads, bool one_hot_nominal, bool encode_missings)->ref<Vectorizer>{
            return ref<Vectorizer>(new Vectorizer(max_heads, one_hot_nominal, encode_missings));
        }), "max_heads"_a=0, "one_hot_nominal"_a=true, "encode_missings"_a=false,
        nb::rv_policy::reference)
    .def("apply", &py_Vectorizer_apply, 
        nb::rv_policy::reference)
    ;
}

