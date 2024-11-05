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

int total_allocated_objs = 0 ;

struct Dog : public CRE_Obj{
    std::string name;
    int         age;

    Dog(std::string _name, int _age=1) : 
        CRE_Obj(), name(_name), age(_age) {
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

struct Kennel : public CRE_Obj {
    ref<Dog> dog;

    Kennel() : CRE_Obj() {}

    static ref<Kennel> make(std::string _name, int _age=1){
        total_allocated_objs += 2;
        ref<Kennel> kennel = new Kennel();
        kennel->dog = new Dog(_name, _age);
        // cout << "K dog refcount(2):" << kennel->dog->get_refcount() << endl;
        return kennel; 
    }

    ~Kennel(){
        total_allocated_objs--;
        std::cout << "Kennel of " << dog->name << " Died:  @" << uint64_t(this) << std::endl;
    }
};


NB_MODULE(dummer_ext, m) {
    nb::class_<CRE_Obj>(m, "CRE_Obj");

    nb::class_<Dog>(m, "Dog")
        .def(nb::new_(&Dog::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_rw("name", &Dog::name)
        .def("get_refcount", &CRE_Obj::get_refcount)
    ;

    nb::class_<Kennel>(m, "Kennel")
        .def(nb::new_(&Kennel::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        .def_rw("dog", &Kennel::dog, nb::rv_policy::reference)
        .def("get_refcount", &CRE_Obj::get_refcount)
    ;

    m.def("leaked_objects", [](){return total_allocated_objs;});
}
