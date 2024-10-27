#include <iostream>
#include <string>
#include <atomic>
#include <memory> 
#include <nanobind/nanobind.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/intrusive/ref.h>
#include <nanobind/intrusive/counter.h>
#include <nanobind/stl/string.h>
#include <nanobind/intrusive/counter.inl>

namespace nb = nanobind;
using namespace nb::literals;
using std::unique_ptr;
using std::shared_ptr;
using std::cout;
using std::endl;



class Object {
public:
    // void inc_ref() const noexcept { ++m_ref_count; }

    // void dec_ref() const noexcept {
    //     if (--m_ref_count == 0)
    //         delete this;
    // }
    void inc_ref() noexcept { m_ref_count.inc_ref(); }
    bool dec_ref() noexcept { return m_ref_count.dec_ref(); }

    // Important: must declare virtual destructor
    virtual ~Object() = default;

    void set_self_py(PyObject *self) noexcept {
        m_ref_count.set_self_py(self);
    }

private:
    // mutable std::atomic<size_t> m_ref_count { 0 };
    nb::intrusive_counter m_ref_count ;
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
        cout << "MADE DOGGO:" << name << endl;
    }

    static Dog make(std::string _name, int _age=1){
        return Dog(_name, _age); 
        // name(_name), age(_age) {
    }

    ~Dog(){
        std::cout << "Doggie Died: " << name << "  @" << uint64_t(this) << std::endl;
    }
};


NB_MODULE(my_ext, m) {
    nb::class_<Object>(m, "Object",
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
        }
    );


    nb::class_<Dog>(m, "Dog")
        // .def(nb::new_([]() { return Dog::make("floot", 0); }))
        .def(nb::new_(&Dog::make), "name"_a="fido", "age"_a=1)
        .def("bark", &Dog::bark)
        .def_rw("name", &Dog::name)
    ;
        // .def(nb::init<>())
        // .def(nb::init<const std::string &>())



}
