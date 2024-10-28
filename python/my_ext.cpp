#include <iostream>
#include <string>
#include <atomic>
#include <memory> 
#include <nanobind/nanobind.h>
// #include <nanobind/nb_types.h>
// #include <nanobind/intrusive/ref.h>
// #include <nanobind/intrusive/counter.h>
#include <nanobind/stl/string.h>
// #include <../external/nanobind/src/nb_internals.h>
#include "../include/ref.h"



namespace nb = nanobind;
using namespace nb::literals;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;



class Object {
private:
    mutable std::atomic<int64_t> m_ref_count { 0 };
    mutable PyObject* m_self_pyobj { nullptr };
    // nb::intrusive_counter m_ref_count ;

public:
    void inc_ref() const noexcept { 
        // cout << "incref: " << uint64_t(this) << endl;
        ++m_ref_count; 
    }

    void dec_ref() const noexcept {
        // cout << "decref:" << uint64_t(this) << endl;
        if (--m_ref_count <= 0){
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

    PyObject* self_py() const noexcept {
        return m_self_pyobj;
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

    static ref<Dog> make(std::string _name, int _age=1){
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


    static ref<Kennel> make(std::string _name, int _age=1){
        ref<Kennel> kennel = new Kennel();
        // Dog* _dog = 
        // cout << "K dog refcount:" << _dog->get_refcount() << endl;
        // _dog->inc_ref();
        // _dog->inc_ref();
        // _dog->inc_ref();
        // cout << "K dog refcount(1):" << _dog->get_refcount() << endl;
        kennel->dog = new Dog(_name, _age);
        // cout << "K refcount:" << kennel->get_refcount() << endl;
        cout << "K dog refcount(2):" << kennel->dog->get_refcount() << endl;
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



// auto dummy_mod = NB_MODULE(__dummy__, m) {
//     nb::class_<Object>(m, "Object");
// }
// std::cout << dummy_mod.dummy << std::endl;

NB_MODULE(dummy_ext, m) {
    nb::class_<Object>(m, "Object"
      // ,nb::intrusive_ptr<Object>(
      //     [](Object *o, PyObject *po) noexcept { o->set_self_py(po); })
    )
    .def("get_refcount", &Object::get_refcount)
    ;
}


typedef void (*dealloc_ty)(PyObject *self);
dealloc_ty nb_inst_dealloc = nullptr;

static void
cre_obj_dealloc(PyObject *self)
{
    // PyObject *error_type, *error_value, *error_traceback;

    /* Save the current exception, if any. */
    // PyErr_Fetch(&error_type, &error_value, &error_traceback);
    // nb::detail::inst_dealloc(self);


    // Call 
    // using CastTy = Object*;
    // using Caster = nb::detail::make_caster<CastTy>;
    // Caster caster;

    

    // auto self_handle = nb::handle_t<CastTy>(self);
    // auto self_handle = nb::handle(self);

    // Object* cpp_self;
    // nb::detail::load_i64(self_handle.ptr(), 0xFF, (int64_t*) cpp_self);


    // bool rv = caster.from_python(self_handle.ptr(), (uint8_t) nb::detail::cast_flags::manual, nullptr);
    // bool rv = caster.from_python(self_handle.ptr(),
    //                     ((uint8_t) nb::detail::cast_flags::convert) |
    //                     ((uint8_t) nb::detail::cast_flags::manual),
    //                     0);


    // cout << "RV: " << rv << endl;

    // cout << "CASTED:" << uint64_t(cpp_self) << endl;
    // cout << "CASTED:" << caster.operator nb::detail::cast_t<CastTy>() << endl;

    
    auto self_handle = nb::handle(self);
    Object* cpp_self = (Object*) nb::inst_ptr<Object>(self_handle);

    cout << "~Before Die~:" << cpp_self->get_refcount() << endl;    


    if(nb_inst_dealloc){
        (*nb_inst_dealloc)(self);    
    }
    cpp_self->dec_ref();
    

    /* Restore the saved exception. */
    // PyErr_Restore(error_type, error_value, error_traceback);
}

// PyObject *myclass_tp_add(PyObject *a, PyObject *b) {
//     return PyNumber_Multiply(a, b);
// }

PyType_Slot slots[] = {
    { Py_tp_dealloc, (void *) cre_obj_dealloc },
    { 0, nullptr }
};




NB_MODULE(my_ext, m) {
    nb::class_<Object>(m, "Object"
      // ,nb::intrusive_ptr<Object>(
      //     [](Object *o, PyObject *po) noexcept { o->set_self_py(po); })
    )
    .def("get_refcount", &Object::get_refcount)

    ;

    auto obj_ty_handle = nb::type<Object>();

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
    // nb::class_<ref<Kennel>>(m, "ref<Kennel>");
        // .def("get_refcount", &Kennel::get_refcount);
    nb::class_<Kennel>(m, "Kennel", nb::type_slots(slots))
        // .def(nb::new_([]() { return Dog::make("floot", 0); }))
        .def(nb::new_(&Kennel::make), "name"_a="fido", "age"_a=1, nb::rv_policy::reference)
        // .def("bark", &Dog::bark)
        .def_rw("dog", &Kennel::dog, nb::rv_policy::reference)
        .def("get_refcount", &Kennel::get_refcount)
        .def("inc_ref", &Kennel::inc_ref)
        .def("dec_ref", &Kennel::dec_ref)
    ;
    // nb::detail::ref_registry::add_type<Kennel>();
        // .def(nb::init<>())
        // .def(nb::init<const std::string &>())



}
