#ifndef _CRE_OBJ_H_
#define _CRE_OBJ_H_


#include <atomic> 
#include "../include/ref.h"

// Forward Declarations
class CRE_Obj;
class AllocBuffer;
// extern "C" void CRE_incref(const CRE_Obj* x);
// extern "C" void CRE_addref(const CRE_Obj* x, size_t n);
// extern "C" void CRE_decref(const CRE_Obj* x);
// extern "C" void CRE_subref(const CRE_Obj* x, size_t n);
// extern "C" int64_t CRE_get_refcount(const CRE_Obj* x);

// Alias CRE_dtor_function as pointer to void(void*)
typedef void (*CRE_dtor_function)(const CRE_Obj* ptr);


class CRE_Obj {
private: 

    // --- Members ---

/*
    Thoughts: we could get this down to 16 bytes 
    int32_ refcount
    uint16_t t_id
    uint16_t alloc_buff_ind
    void* proxy_obj
*/

// Refcount 
#ifndef CRE_NONATOMIC_REFCOUNT
    mutable std::atomic_int64_t  ref_count;// = {0};
#else
    mutable int64_t  ref_count = 0;
#endif

public :
    // When using nanobind proxy_object is a PyObject*
    mutable void*               proxy_obj = nullptr; 
    CRE_dtor_function   dtor;
    // ref<AllocBuffer>        alloc_buffer = nullptr;

    // NOTE: Cannot Use ref<AllocBuffer> becuase would 
    //   reference incomplete type AllocBuffer
    AllocBuffer*        alloc_buffer = nullptr;

    // --- Methods ---
    CRE_Obj(CRE_dtor_function dtor = nullptr);
    // CRE_Obj();
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


    inline bool _check_destroy() const noexcept{

        if (ref_count < 0) [[unlikely]] {
            std::cout << "Warning: refcounting underflow!" << std::endl;
        }
        if (ref_count <= 0) {
            // delete this;
            this->dtor(this);
            return true;
        }
        return false;
    }
    inline void inc_ref() const noexcept{
        #ifndef CRE_NONATOMIC_REFCOUNT
            ref_count.fetch_add(1, std::memory_order_relaxed);
        #else
            ref_count++; 
        #endif
    }
    inline void add_ref(size_t n) const noexcept{
        #ifndef CRE_NONATOMIC_REFCOUNT
            ref_count.fetch_add(n, std::memory_order_relaxed);
        #else
            ref_count += n; 
        #endif
    }
    inline bool dec_ref() const noexcept{
        #ifndef CRE_NONATOMIC_REFCOUNT
            ref_count.fetch_sub(1, std::memory_order_relaxed);
        #else
            ref_count--;
        #endif

        return _check_destroy();
    }
    inline bool sub_ref(size_t n) const noexcept{
        // cout << "INCREF" << endl;
        #ifndef CRE_NONATOMIC_REFCOUNT
            ref_count.fetch_sub(n, std::memory_order_relaxed);
        #else
            ref_count -= n; 
        #endif

        return _check_destroy();
    }

    // void operator delete(void * p);
};

const uint8_t LOW_VERBOSITY = 1;
const uint8_t DEFAULT_VERBOSITY = 2;
const uint8_t HIGH_VERBOSITY = 3;



#endif /* _CRE_OBJ_H_ */
