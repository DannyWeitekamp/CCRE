#include "../include/py_cre_core.h"


// Forward Declare various modules
void init_core(nb::module_ & m);
void init_fact(nb::module_ & m);
void init_factset(nb::module_ & m);
void init_func(nb::module_ & m);
void init_var(nb::module_ & m);
void init_flattener(nb::module_ & m);
void init_vectorizer(nb::module_ & m);


NB_MODULE(cre, m) {
    init_core(m);
    init_fact(m);
    init_factset(m);
    init_func(m);
    init_var(m);
    init_flattener(m);
    init_vectorizer(m);
    
}
