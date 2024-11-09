#ifndef _CRE_OBJ_H_
#define _CRE_OBJ_H_


#include <atomic> 
#include "../include/ref.h"

// Forward Declarations
class CRE_Obj;
class AllocBuffer;
extern "C" void CRE_incref(const CRE_Obj* x);
extern "C" void CRE_addref(const CRE_Obj* x, size_t n);
extern "C" void CRE_decref(const CRE_Obj* x);
extern "C" void CRE_subref(const CRE_Obj* x, size_t n);
extern "C" int64_t CRE_get_refcount(const CRE_Obj* x);

// Alias CRE_dtor_function as pointer to void(void*)
typedef void (*CRE_dtor_function)(const CRE_Obj* ptr);


class CRE_Obj {
private: 

    // --- Members ---
// Refcount 
#ifndef CRE_NONATOMIC_REFCOUNT
    mutable std::atomic_int64_t  ref_count;// = {0};
#else
    mutable int64_t  ref_count = 0;
#endif

public :
    // When using nanobind proxy_object is a PyObject*
    mutable void*               proxy_obj = nullptr; 
    // CRE_dtor_function   dtor;
    // ref<AllocBuffer>        alloc_buffer = nullptr;

    // NOTE: Cannot Use ref<AllocBuffer> becuase would 
    //   reference incomplete type AllocBuffer
    AllocBuffer*        alloc_buffer = nullptr;

    // --- Methods ---
    // CRE_Obj(CRE_dtor_function dtor = NULL);
    CRE_Obj();
    // ~CRE_Obj(){};
    int64_t get_refcount() noexcept;

    // Note: We could use virtual destructors, but it seems to slow
    //  down builing facts by about ~30%, probably because it makes
    //  them into polymorphic types. It isn't clear that there really
    //  is a need for polymorphic types in CRE. For instance, all fact
    //  specializations follow a pretty generic implementation.
    // virtual ~CRE_Obj() = default;
    
    // Friends
    // friend void CRE_incref(const CRE_Obj* x);
    // friend void CRE_addref(const CRE_Obj* x, size_t n);
    // friend void _check_destroy(const CRE_Obj* x);
    // friend void CRE_decref(const CRE_Obj* x);
    // friend void CRE_subref(const CRE_Obj* x, size_t n);
    // friend int64_t CRE_get_refcount(const CRE_Obj* x);


    inline bool _check_destroy(){
        if (x->ref_count <= 0) {
            delete x;
            return true;
        }
        return false;
    }
    inline void inc_ref(){
        #ifndef CRE_NONATOMIC_REFCOUNT
            x->ref_count.fetch_add(1, std::memory_order_relaxed);
        #else
            x->ref_count++; 
        #endif
    }
    inline void add_ref(){
        #ifndef CRE_NONATOMIC_REFCOUNT
            x->ref_count.fetch_add(n, std::memory_order_relaxed);
        #else
            x->ref_count += n; 
        #endif
    }
    inline bool dec_ref(){
        #ifndef CRE_NONATOMIC_REFCOUNT
            x->ref_count.fetch_sub(1, std::memory_order_relaxed);
        #else
            x->ref_count--;
        #endif

        return _check_destroy(x);
    }
    inline bool sub_ref(){
        // cout << "INCREF" << endl;
        #ifndef CRE_NONATOMIC_REFCOUNT
            x->ref_count.fetch_sub(n, std::memory_order_relaxed);
        #else
            x->ref_count -= n; 
        #endif

        return _check_destroy(x);
    }

    void operator delete(void * p);
};

#endif /* _CRE_OBJ_H_ */
