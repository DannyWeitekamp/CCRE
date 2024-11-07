#include <iostream>
#include <string>
#include <atomic>
#include <memory> 

// Note: Need to be included in this order
#include <nanobind/nanobind.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>
#include <nanobind/intrusive/counter.inl>
#include <nanobind/stl/string.h>

namespace nb = nanobind;
using namespace nb::literals;
using nb::ref;
using nb::intrusive_base;

int total_allocated_objs = 0 ;

class Object : public intrusive_base {
public:
    Object () : intrusive_base() {
        // std::cout << "THIS REFCOUNT:" << this->get_refcount() << std::endl;
    }
    uint64_t get_refcount() {
        // Needed to edit 
        return intrusive_base::get_refcount();
    }
};

struct Dog : public Object{
    std::string name;
    int         age;

    Dog(std::string _name, int _age=1) : 
        Object(), name(_name), age(_age) {
        total_allocated_objs++;
    }

    static ref<Dog> make(std::string _name, int _age=1){
        total_allocated_objs++;
        return new Dog(_name, _age); 
    }

    ~Dog(){
        total_allocated_objs--;
        std::cout << "Doggie Died: " << name << "  @" << uint64_t(this) << std::endl;
    }
};

struct Kennel : public Object {
    ref<Dog> dog;

    Kennel() : Object() {}
    Kennel(std::string _name, int _age) : Object() {
        total_allocated_objs++;
        dog = new Dog(_name, _age);
    }
    static ref<Kennel> make(std::string _name, int _age=1){
        total_allocated_objs++;
        Kennel* kennel = new Kennel();
        kennel->dog = new Dog(_name, _age);
        // std::cout << "K dog refcount(2):" << kennel->dog->get_refcount() << std::endl;
        return kennel; 
    }


    ~Kennel(){
        total_allocated_objs--;
        std::cout << "Kennel of " << dog->name << " Died:  @" << uint64_t(this) << std::endl;
    }
};


NB_MODULE(dummer_ext, m) {
    nb::class_<Object>(
      m, "Object",
      nb::intrusive_ptr<Object>(
          [](Object *o, PyObject *po) noexcept { o->set_self_py(po); })
      );

    nb::intrusive_init(
    [](PyObject *o) noexcept {
        nb::gil_scoped_acquire guard;
        Py_INCREF(o);
    },
    [](PyObject *o) noexcept {
        nb::gil_scoped_acquire guard;
        Py_DECREF(o);
    });

    nb::class_<Dog>(m, "Dog")
        .def(nb::init<std::string, int64_t>(), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        // .def(nb::new_(&Dog::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_static("make", &Dog::make, "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_rw("name", &Dog::name)
        .def("get_refcount", &Object::get_refcount)
    ;

    nb::class_<Kennel>(m, "Kennel")
        .def(nb::init<std::string, int64_t>(), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        // .def(nb::new_(&Kennel::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_static("make", &Kennel::make, "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_rw("dog", &Kennel::dog, nb::rv_policy::reference)
        .def("get_refcount", &Object::get_refcount)
    ;

    m.def("leaked_objects", [](){return total_allocated_objs;});
}
