#ifndef _CRE_OBJ_H_
#define _CRE_OBJ_H_


#include <atomic> 

// Forward Declarations
class CRE_Obj;
class AllocBuffer;
extern "C" void CRE_incref(CRE_Obj* x);
extern "C" void CRE_addref(CRE_Obj* x, size_t n);
extern "C" void CRE_decref(CRE_Obj* x);
extern "C" void CRE_subref(CRE_Obj* x, size_t n);
extern "C" size_t CRE_get_refcount(CRE_Obj* x);

// Alias CRE_dtor_function as pointer to void(void*)
typedef void (*CRE_dtor_function)(CRE_Obj* ptr);


class CRE_Obj {
private: 

    // --- Members ---
// Refcount 
#ifndef CRE_NONATOMIC_REFCOUNT
    std::atomic_size_t  ref_count;
#else
    size_t  ref_count;
#endif

public :
    CRE_dtor_function   dtor;
    AllocBuffer*        alloc_buffer;

    // --- Methods ---
    CRE_Obj(CRE_dtor_function dtor = NULL);
    // ~CRE_Obj();
    size_t get_refcount();
    
    // Friends
    friend void CRE_incref(CRE_Obj* x);
    friend void CRE_addref(CRE_Obj* x, size_t n);
    friend void _check_destroy(CRE_Obj* x);
    friend void CRE_decref(CRE_Obj* x);
    friend void CRE_subref(CRE_Obj* x, size_t n);
    friend size_t CRE_get_refcount(CRE_Obj* x);
};

#endif /* _CRE_OBJ_H_ */
