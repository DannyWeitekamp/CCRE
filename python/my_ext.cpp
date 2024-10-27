#include <iostream>
#include <string>
#include <atomic>
#include <memory> 
#include <nanobind/nanobind.h>
// #include <nanobind/intrusive/ref.h>
// #include <nanobind/intrusive/counter.h>
#include <nanobind/stl/string.h>
#include "../include/ref.h"



namespace nb = nanobind;
using namespace nb::literals;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;



class Object {
private:
    mutable std::atomic<size_t> m_ref_count { 0 };
    // nb::intrusive_counter m_ref_count ;

public:
    void inc_ref() const noexcept { 
        // cout << "incref: " << uint64_t(this) << endl;
        ++m_ref_count; 
    }

    void dec_ref() const noexcept {
        // cout << "decref:" << uint64_t(this) << endl;
        if (--m_ref_count == 0){
            // cout << "delete" << endl;
            delete this;
            // return true;
        }
        // return false;
    }
    // void inc_ref() noexcept { m_ref_count.inc_ref(); }
    // bool dec_ref() noexcept { return m_ref_count.dec_ref(); }

    // Important: must declare virtual destructor
    virtual ~Object() = default;

    // void set_self_py(PyObject *self) noexcept {
    //     m_ref_count.set_self_py(self);
    // }

    int get_refcount() noexcept {
        // return int(*(int*)&m_ref_count);
        return int(m_ref_count);
    }
};

// // Convenience function for increasing the reference count of an instance
// inline void inc_ref(Object *o) noexcept {
//     if (o)
//        o->inc_ref();
// }

// // Convenience function for decreasing the reference count of an instance
// // and potentially deleting it when the count reaches zero
// inline void dec_ref(Object *o) noexcept {
//     if (o && o->dec_ref())
//         delete o;
// }



// #include <nanobind/intrusive/ref.h>
// #include <nanobind/intrusive/counter.inl>


//-------------------------------------------------------
// : Dog

struct Dog : public Object{
    std::string name;
    int         age;

    std::string bark() const {
        return name + ": woof!";
    }

    Dog(std::string _name, int _age=1) : 
        name(_name), age(_age) {
        // cout << "MADE DOGGO:" << name << endl;
    }

    static Dog* make(std::string _name, int _age=1){
        return new Dog(_name, _age); 
        // name(_name), age(_age) {
    }

    ~Dog(){
        std::cout << "Doggie Died: " << name << "  @" << uint64_t(this) << std::endl;
    }
};

struct Kennel : public Object {
    ref<Dog> dog;
    // Dog* dog;


    static Kennel* make(std::string _name, int _age=1){
        Kennel* kennel = new Kennel();
        Dog* _dog = new Dog(_name, _age);
        cout << "K dog refcount:" << _dog->get_refcount() << endl;
        // _dog->inc_ref();
        // _dog->inc_ref();
        // _dog->inc_ref();
        cout << "K dog refcount:" << _dog->get_refcount() << endl;
        kennel->dog = _dog;
        cout << "K dog refcount:" << kennel->dog->get_refcount() << endl;
        return kennel; 
        // name(_name), age(_age) {
    }

    ~Kennel(){
        std::cout << "Kennel of " << dog->name << " Died:  @" << uint64_t(this) << std::endl;
    }
};

// void del_creobj_proxy(PyObject*){
    
// }

// PyObject* add_delete(PyObject*){
//     ((PyTypeObject *) result)->tp_vectorcall = type_vectorcall;
// }


NB_MODULE(my_ext, m) {
    nb::class_<Object>(m, "Object"
      // ,nb::intrusive_ptr<Object>(
      //     [](Object *o, PyObject *po) noexcept { o->set_self_py(po); })
    );

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


    nb::class_<Dog>(m, "Dog")
        // .def(nb::new_([]() { return Dog::make("floot", 0); }))
        .def(nb::new_(&Dog::make), "name"_a="fido", "age"_a=1)
        .def("bark", &Dog::bark)
        .def_rw("name", &Dog::name)
        .def("get_refcount", &Kennel::get_refcount)
        .def("inc_ref", &Dog::inc_ref)
        .def("dec_ref", &Dog::dec_ref)
        .def("__del__",
           +[](const Dog&) -> void { 
               std::cerr << "deleting C" << std::endl;
           }
        );

        // .def_destructor([](Dog* self) {
        //     if(self) self->dec_ref();
        // })

    ;
    // nb::detail::ref_registry::add_type<Dog>();

    nb::class_<Kennel>(m, "Kennel")
        // .def(nb::new_([]() { return Dog::make("floot", 0); }))
        .def(nb::new_(&Kennel::make), "name"_a="fido", "age"_a=1)
        // .def("bark", &Dog::bark)
        .def_rw("dog", &Kennel::dog)
        .def("get_refcount", &Kennel::get_refcount)
        .def("inc_ref", &Kennel::inc_ref)
        .def("dec_ref", &Kennel::dec_ref)
    ;
    // nb::detail::ref_registry::add_type<Kennel>();
        // .def(nb::init<>())
        // .def(nb::init<const std::string &>())



}
