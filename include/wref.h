/*
    nanobind/intrusive/ref.h: This file defines the ``ref<T>`` RAII scoped
    reference counting helper class.

    When included following ``nanobind/nanobind.h``, the code below also
    exposes a custom type caster to bind functions taking or returning
    ``ref<T>``-typed values.

    Copyright (c) 2023 Wenzel Jakob

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once
#include <iostream>
#include "../include/ref.h"
#include "../include/cre_obj.h"

#if defined(NB_VERSION_MAJOR)
#include "../external/nanobind/src/nb_internals.h"
#endif
// #include "counter.h"

// namespace cre {

// struct CRE_Obj;

// } // NAMESPACE_END(cre)

// NAMESPACE_BEGIN(nanobind)

using cre::ControlBlock;

/**
 * \brief RAII scoped reference counting helper class
 *
 * ``ref`` is a simple RAII wrapper class that encapsulates a pointer to an
 * instance with intrusive reference counting.
 *
 * It takes care of increasing and decreasing the reference count as needed and
 * deleting the instance when the count reaches zero.
 *
 * For this to work, compatible functions ``inc_wref()`` and ``dec_wref()`` must
 * be defined before including this file. Default implementations for
 * subclasses of the type ``intrusive_base`` are already provided as part of the
 * file ``counter.h``.
 */
template <typename T> class wref {
private:
    ControlBlock* m_ptr = nullptr;
public:
    /// Create a null reference
    wref() = default;

    /// Construct a reference from a pointer
    wref(T *ptr) : m_ptr(ptr->control_block) { 
        // std::cout << "constr from ptr" << std::endl;

        if(m_ptr) m_ptr->inc_wref();
    }

    /// Copy a weak reference, increases the reference count
    wref(const wref &r) : m_ptr(r.control_block) {
        // std::cout << "copy from ref" << std::endl;
        if(m_ptr) m_ptr->inc_wref();
    }

    /// Copy a strong reference, increases the weak reference count
    wref(const ref<T> &r) : m_ptr(r.get()->control_block) {
        // std::cout << "copy from ref" << std::endl;
        if(m_ptr) m_ptr->inc_wref();
    }

    /// Move a weak reference witout changing the reference count
    wref(wref &&r) noexcept : m_ptr(r.m_ptr) { r.m_ptr = nullptr; }

    /// Destroy this reference
    ~wref() { 
        // std::cout << "destroy " << std::endl;
        if(m_ptr) m_ptr->dec_wref();
    }

    /// Move-assign another reference into this one
    wref &operator=(wref &&r) noexcept {
        // std::cout << "move" << std::endl;
        if(m_ptr) m_ptr->dec_wref();
        m_ptr = r.m_ptr;
        r.m_ptr = nullptr;
        return *this;
    }

    /// Copy-assign another reference into this one
    wref &operator=(const wref &r) {
        // std::cout << "& OP ver const" << std::endl;
        if(r.m_ptr) r.m_ptr->inc_wref();
        if(m_ptr) m_ptr->dec_wref();
        m_ptr = r.m_ptr;
        return *this;
    }

    /// Overwrite this reference with a pointer to another object
    wref &operator=(T *ptr) {
        // std::cout << "& OP ver" << std::endl;
        if(ptr) ptr->inc_wref();
        // std::cout << "&&" << std::endl;
        if(m_ptr) m_ptr->dec_wref();
        // std::cout << "**" << std::endl;
        m_ptr = (ptr == nullptr) ? nullptr : ptr->control_block;
        // std::cout << "end" << std::endl;
        return *this;
    }

    /// Clear the currently stored reference
    void reset() {
        if(m_ptr) m_ptr->dec_wref();
        m_ptr = nullptr;
    }

    inline T* _get_obj_ptr() const{
        if(m_ptr && m_ptr->wref_count & 1){
            return (T*) m_ptr->obj_ptr;
        }else{
            return nullptr;
        }
    }

    /// Compare this reference with another weak reference
    bool operator==(const wref &r) const { return m_ptr == r.m_ptr; }
    bool operator!=(const wref &r) const { return m_ptr != r.m_ptr; }
    
    /// Compare this reference with a strong reference
    bool operator==(const ref<T> &r) const { return _get_obj_ptr() == r.m_ptr; }
    bool operator!=(const ref<T> &r) const { return _get_obj_ptr() != r.m_ptr; }

    /// Compare this reference with a pointer
    bool operator==(const T *ptr) const { return _get_obj_ptr() == ptr; }
    bool operator!=(const T *ptr) const { return _get_obj_ptr() != ptr; }

    /// Access the object referenced by this reference
    T *operator->() { return _get_obj_ptr(); }

    /// Access the object referenced by this reference
    const T *operator->() const { return _get_obj_ptr(); }

    /// Return a C++ reference to the referenced object
    T &operator*() { return *(_get_obj_ptr()); }

    /// Return a const C++ reference to the referenced object
    const T &operator*() const { return *(_get_obj_ptr()); }

    /// Return a pointer to the referenced object
    operator T *() { return _get_obj_ptr(); }

    /// Return a const pointer to the referenced object
    operator const T *() const { return _get_obj_ptr(); }

    /// Return a pointer to the referenced object
    T *get() { return _get_obj_ptr(); }

    /// Return a const pointer to the referenced object
    const T *get() const { return _get_obj_ptr(); }

    /// Return a pointer to the referenced object's control_block
    ControlBlock *get_cb() { return m_ptr; }

    /// Return a const pointer to the referenced object's control_block
    const ControlBlock *get_cb() const { return m_ptr; }

    int64_t get_wrefcount() const {
        if(m_ptr){
            return m_ptr->get_wrefcount();    
        }else{
            return 0;
        }
    }

    int64_t get_refcount() const {
        if(m_ptr && m_ptr->wref_count & 1){
            return m_ptr->obj_ptr->get_refcount();
        }else{
            return 0;
        }
    }
};

// } // NAMESPACE_END(cre)

// #include <nanobind/nanobind.h>
// Registar a type caster for ``wref<T>`` if nanobind was previously #included
#if defined(NB_VERSION_MAJOR)
NAMESPACE_BEGIN(nanobind)
NAMESPACE_BEGIN(detail)
template <typename T> struct type_caster<wref<T>> {
    using Caster = make_caster<T>;
    static constexpr bool IsClass = true;
    NB_TYPE_CASTER(wref<T>, Caster::Name)

    bool from_python(handle src, uint8_t flags,
                     cleanup_list *cleanup) noexcept {

        // std::cout << "INP IS READY: " << inst_ready(src) << std::endl;
        Caster caster;
        if (!caster.from_python(src, flags, cleanup))
            return false;

        value = Value(caster.operator T *());
        return true;
    }

    static handle from_cpp(const wref<T> &value, rv_policy policy,
                           cleanup_list *cleanup) noexcept {
        // if constexpr (std::is_base_of_v<intrusive_base, T>)
        const T* cpp_obj = value.get();

        if(cpp_obj == nullptr){
            throw std::bad_weak_ptr("Attempt to return a freed CRE_Obj to python.");
        }

        const ControlBlock* control_block = value.get_cb();
        if (policy != rv_policy::copy && policy != rv_policy::move){
            if (PyObject* obj = (PyObject*) control_block->proxy_obj){
                return handle(obj).inc_ref();
            }else{
                // NOTE: A Python proxy from a weakref should still hold a 
                //  strong reference, otherwise things could get segfaulty.
                cpp_obj->inc_ref();
            }
        }
        handle py_out = Caster::from_cpp(value.get(), policy, cleanup);
        nb_inst* out_inst = (nb_inst*) py_out.ptr();
        out_inst->unused = 17;
        // std::cout << "CPP DELETE: "  << out_inst->cpp_delete << std::endl; 
        // std::cout << "INTRUSIVE: "  << out_inst->intrusive << std::endl; 
        // auto _is = inst_state(py_out);
        // std::cout << "-- " << cast<std::string>(str(py_out)) << " STATE: ready=" <<  _is.first << ", destruct=" <<  _is.second << std::endl;
        // std::cout << "STATE: " << _is.first << ", " << _is.second << std::endl;
        cpp_obj->control_block->proxy_obj = (void*) py_out.ptr();
        return py_out;
    }
};
NAMESPACE_END(detail)
NAMESPACE_END(nanobind)
#endif


