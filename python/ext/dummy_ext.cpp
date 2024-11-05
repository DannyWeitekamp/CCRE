#include <iostream>
#include <string>
#include <atomic>
#include <memory> 

// Note: Need to be included in this order
#include <nanobind/nanobind.h>
#include "../../include/cre_obj.h"
#include "../../include/ref.h"
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;


struct Dog : public CRE_Obj{
    std::string name;
    int         age;

    std::string bark() const {
        return name + ": woof!";
    }

    Dog(std::string _name, int _age=1) : 
        name(_name), age(_age) {
    }

    static ref<Dog> make(std::string _name, int _age=1){
        return new Dog(_name, _age); 
    }

    ~Dog(){
        std::cout << "Doggie Died: " << name << "  @" << uint64_t(this) << std::endl;
    }
};

struct Kennel : public CRE_Obj {
    ref<Dog> dog;
    static ref<Kennel> make(std::string _name, int _age=1){
        ref<Kennel> kennel = new Kennel();
        kennel->dog = new Dog(_name, _age);
        // cout << "K dog refcount(2):" << kennel->dog->get_refcount() << endl;
        return kennel; 
    }

    ~Kennel(){
        std::cout << "Kennel of " << dog->name << " Died:  @" << uint64_t(this) << std::endl;
    }
};


typedef void (*dealloc_ty)(PyObject *self);
dealloc_ty nb_inst_dealloc = nullptr;

static void cre_obj_dealloc(PyObject *self){
    auto self_handle = nb::handle(self);
    CRE_Obj* cpp_self = (CRE_Obj*) nb::inst_ptr<CRE_Obj>(self_handle);

    // cout << "~Before Die~:" << cpp_self->get_refcount() << endl;    

    if(nb_inst_dealloc){
        (*nb_inst_dealloc)(self);
    }
    // When the Python proxy object is collected remove 
    //   the C++ object's reference to it so it isn't reused
    cpp_self->proxy_obj = nullptr;
    CRE_decref(cpp_self);//->dec_ref();
}

// PyObject *myclass_tp_add(PyObject *a, PyObject *b) {
//     return PyNumber_Multiply(a, b);
// }

PyType_Slot slots[] = {
    { Py_tp_dealloc, (void *) cre_obj_dealloc },
    { 0, nullptr }
};




NB_MODULE(dummy_ext, m) {
    nb::class_<CRE_Obj>(m, "CRE_Obj"
      // ,nb::intrusive_ptr<Object>(
      //     [](Object *o, PyObject *po) noexcept { o->set_self_py(po); })
    )
    .def("get_refcount", &CRE_Obj::get_refcount)
    ;

    auto obj_ty_handle = nb::type<CRE_Obj>();

    if (obj_ty_handle.is_valid()) {
        PyTypeObject* obj_ty = (PyTypeObject*) obj_ty_handle.ptr();

        std::cout << "PRINTED: " << uint64_t(obj_ty) << std::endl;
        
        std::cout << "OBJ DEALLOC: " << uint64_t(obj_ty->tp_dealloc) << std::endl;
        nb_inst_dealloc = obj_ty->tp_dealloc;
        // Do something with the type object, e.g., print its name
        // py::print(my_class_type.attr("__name__"));
    }
    // nb::intrusive_init(
    //     [](PyObject *o) noexcept {
    //         nb::gil_scoped_acquire guard;
    //         Py_INCREF(o);
    //     },
    //     [](PyObject *o) noexcept {
    //         nb::gil_scoped_acquire guard;
    //         Py_DECREF(o);
    //     }
    // );

    // nb::class_<ref<Dog>>(m, "ref<Dog>");

nb::class_<Dog>(m, "Dog", nb::type_slots(slots))
    // .def(nb::new_([]() { return Dog::make("floot", 0); }))
    .def(nb::new_(&Dog::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
    .def("bark", &Dog::bark)
    .def_rw("name", &Dog::name)
    .def("get_refcount", &Dog::get_refcount)
        // .def("inc_ref", &Dog::inc_ref)
        // .def("dec_ref", &Dog::dec_ref)
        // .def("__del__",
        //    +[](const Dog&) -> void { 
        //        std::cerr << "deleting C" << std::endl;
        //    }
        // );

        // .def_destructor([](Dog* self) {
        //     if(self) self->dec_ref();
        // })

    ;
    // nb::detail::ref_registry::add_type<Dog>();
    // nb::class_<ref<Kennel>>(m, "ref<Kennel>");
        // .def("get_refcount", &Kennel::get_refcount);
    nb::class_<Kennel>(m, "Kennel", nb::type_slots(slots))
        // .def(nb::new_([]() { return Dog::make("floot", 0); }))
        .def(nb::new_(&Kennel::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        // .def("bark", &Dog::bark)
        .def_rw("dog", &Kennel::dog, nb::rv_policy::reference)
        .def("get_refcount", &Kennel::get_refcount)
        // .def("inc_ref", &Kennel::inc_ref)
        // .def("dec_ref", &Kennel::dec_ref)
    ;
    // nb::detail::ref_registry::add_type<Kennel>();
        // .def(nb::init<>())
        // .def(nb::init<const std::string &>())



}
