#ifndef _CRE_OBJ_H_
#define _CRE_OBJ_H_


#include <atomic> 

// Alias CRE_dtor_function as pointer to void(void*)
typedef void (*CRE_dtor_function)(void *ptr);

// Forward Declarations
class CRE_Obj;
extern "C" void CRE_incref(CRE_Obj* x);
extern "C" void CRE_decref(CRE_Obj* x);
extern "C" size_t CRE_get_refcount(CRE_Obj* x);


class CRE_Obj {
private: 

// Refcount 
#ifndef CRE_NONATOMIC_REFCOUNT
    std::atomic_size_t  ref_count;
#else
    size_t  ref_count;
#endif

public :
    CRE_dtor_function   dtor;

    CRE_Obj(CRE_dtor_function dtor = NULL);
    ~CRE_Obj();
    size_t get_refcount();
    
    // Friends
    friend void CRE_incref(CRE_Obj* x);
    friend void CRE_decref(CRE_Obj* x);
    friend size_t CRE_get_refcount(CRE_Obj* x);
};

#endif /* _CRE_OBJ_H_ */
