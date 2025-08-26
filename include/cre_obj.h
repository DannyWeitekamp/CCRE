#pragma once

#include <atomic> 
#include "../include/ref.h"
// #include "../include/alloc_buffer.h"
// #include "../include/context.h"
// #include "../include/control_block.h"

namespace cre {

using std::cout;
using std::endl;

// Forward Declarations
// class ControlBlock;
class CRE_Obj;
struct AllocBuffer;
// extern "C" void CRE_incref(const CRE_Obj* x);
// extern "C" void CRE_addref(const CRE_Obj* x, size_t n);
// extern "C" void CRE_decref(const CRE_Obj* x);
// extern "C" void CRE_subref(const CRE_Obj* x, size_t n);
// extern "C" int64_t CRE_get_refcount(const CRE_Obj* x);

// Alias CRE_dtor_function as pointer to void(void*)
typedef void (*CRE_dtor_function)(const CRE_Obj* ptr);

// const char* EMPTY_STR = "";

/* 
    Similar to an std::shared_ptr control block yet designed to  
    complement the intrusive reference counting of a CRE_Obj. In most cases, 
    the ControlBlock remains mostly untouched. It is not the sole means by which
    the CRE_Obj is accessed. Unlike a shared_ptr this class only participates in
    weak reference counting, from wref instances. The lowest bit of wref_count is 1
    until the CRE_Obj's strong reference count reaches zero. Also, ControlBlocks 
    hold some extra data that is usually only relevant to a CRE_Obj at its 
    creation and destruction. This offloads a great deal of cache
    pressure for subroutines that access lots of CRE_Obj instances. 
    ControlBlocks are almost always pool allocated with ControlBlockPool.    
*/
struct ControlBlock {

    // ControlBlock* next; 
    CRE_Obj* obj_ptr;

    // The wref_count increments by 2 and the lowest bit is 1 when
    //  the strong count is non-zero and 0 when it is zero.  
    #ifndef CRE_NONATOMIC_REFCOUNT
        // mutable std::atomic_uint32_t  ref_count;// = {0};
        mutable std::atomic_int64_t  wref_count = {1};
    #else
        // mutable uint32_t  ref_count = 0;
        mutable int64_t  wref_count = 1;
    #endif

    uint16_t t_id;
    uint16_t pad[3];


    // C-String of unique identifier for the CRE_Obj pointed to
    //  we avoid std::string here since we really only need
    //  this for printing purposes, and it should be valid
    //  for interned strings and objects with normal lifecycles.
    char* unique_id = nullptr;


    // When using nanobind proxy_object is a PyObject*
    void*               proxy_obj = nullptr; 
    CRE_dtor_function   dtor;
    // ref<AllocBuffer>        alloc_buffer = nullptr;

    // NOTE: Cannot Use ref<AllocBuffer> becuase would 
    //   reference incomplete type AllocBuffer
    AllocBuffer*        alloc_buffer = nullptr;


    ControlBlock(CRE_Obj* obj_ptr, CRE_dtor_function _dtor, uint16_t t_id=0);
    ~ControlBlock();
        // cout << "DESRTROY CB" << endl;
    

    friend class CRE_Obj;

    inline bool _check_destroy() const noexcept{
        if (wref_count < 0) [[unlikely]] {
            cout << "Warning: weak ref_count underflow!" << endl;
        }
        if (wref_count <= 0) {
            // cout << "destroy CB: " << uint64_t(this) << endl;
            // delete this;
            // global_cb_pool.dealloc(this);
            this->~ControlBlock();
            // ::operator delete((void*) this);

            return true;
        }
        return false;
    }
    inline void inc_wref() const noexcept{
        #ifndef CRE_NONATOMIC_REFCOUNT
            wref_count.fetch_add(2, std::memory_order_relaxed);
        #else
            wref_count += 2; 
        #endif

        // cout << "INCR W: " << wref_count << " " << uint64_t(this) <<  endl;
    }
    inline void add_wref(size_t n) const noexcept{
        
        #ifndef CRE_NONATOMIC_REFCOUNT
            wref_count.fetch_add(n<<1, std::memory_order_relaxed);
        #else
            wref_count += n<<1; 
        #endif
        // cout << "ADD W: " << wref_count << " " << uint64_t(this) <<  endl;
    }
    inline bool dec_wref() const noexcept{
        

        #ifndef CRE_NONATOMIC_REFCOUNT
            wref_count.fetch_sub(2, std::memory_order_relaxed);
        #else
            wref_count -= 2;
        #endif

        // cout << "DECR W: " << wref_count << " " << uint64_t(this) <<  endl;

        return _check_destroy();
    }
    inline bool sub_wref(size_t n) const noexcept{
        
        #ifndef CRE_NONATOMIC_REFCOUNT
            wref_count.fetch_sub(n<<1, std::memory_order_relaxed);
        #else
            wref_count -= n<<1; 
        #endif

        // cout << "SUB W: " << wref_count << " " << uint64_t(this) <<  endl;

        return _check_destroy();
    }

    inline int64_t get_wrefcount() noexcept {
        return wref_count >> 1;
    }

    inline bool expire() noexcept {    
        wref_count = wref_count & ~1;
        return _check_destroy();   
    }

    inline bool is_expired() noexcept {
        return !(wref_count & 1);
    }
};


// class ControlBlockPool {
// private:
//     size_t block_size;
//     size_t chunks_per_block;
//     ControlBlock* alloc_ptr = nullptr;
    
// public:
//     ControlBlockPool(size_t n_);
//     ControlBlock* alloc();
//     void          dealloc(ControlBlock* ptr);

// private:
//     ControlBlock* alloc_block();
// };


// global_con


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
    // mutable std::atomic_int32_t  wref_count;// = {0};
#else
    mutable uint64_t  ref_count = 0;
    // mutable uint32_t  wref_count = 0;
#endif

// #ifndef CRE_NONATOMIC_REFCOUNT
//     mutable std::atomic_int64_t  ref_count;// = {0};
// #else
//     mutable int64_t  ref_count = 0;
// #endif

public :
    mutable ControlBlock* control_block;

    // When using nanobind proxy_object is a PyObject*
    // mutable void*               proxy_obj = nullptr; 
    // CRE_dtor_function   dtor;
    // ref<AllocBuffer>        alloc_buffer = nullptr;

    // NOTE: Cannot Use ref<AllocBuffer> becuase would 
    //   reference incomplete type AllocBuffer
    // AllocBuffer*        alloc_buffer = nullptr;

    // --- Methods ---
    void init_control_block(CRE_dtor_function _dtor, uint16_t _t_id=0);
    // CRE_Obj(CRE_dtor_function dtor = nullptr);
    // CRE_Obj();
    // ~CRE_Obj(){};
    int64_t get_refcount() noexcept;
    int64_t get_wrefcount() noexcept;

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
            std::cout << "Warning: ref_count underflow!" << std::endl;
        }
        if (ref_count <= 0) {
            // cout << "DESTROY S" << endl;
            
            // this->control_block->wref_count = this->control_block->wref_count & ~1;
            // Call the CRE_Obj's destructor
            ControlBlock* cb = this->control_block;
            if(cb->dtor){
                cb->dtor(this);    
            }

            return cb->expire();
        }
        return false;
    }
    inline void inc_ref() const noexcept{
        #ifndef CRE_NONATOMIC_REFCOUNT
            ref_count.fetch_add(1, std::memory_order_relaxed);
        #else
            ref_count++; 
        #endif
        // cout << "INCR S: " << ref_count << " " << uint64_t(this) <<  endl;
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

        // cout << "DECR S: " << ref_count << " " << uint64_t(this) <<  endl;

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

    inline void inc_wref() const noexcept{
        this->control_block->inc_wref();
    }
    inline void add_wref(size_t n) const noexcept{
        this->control_block->add_wref(n);
    }
    inline void dec_wref() const noexcept{
        this->control_block->dec_wref();
    }
    inline void sub_wref(size_t n) const noexcept{
        this->control_block->sub_wref(n);
    }
    // void operator delete(void * p);
};

const uint8_t LOW_VERBOSITY = 1;
const uint8_t DEFAULT_VERBOSITY = 2;
const uint8_t HIGH_VERBOSITY = 3;


std::tuple<CRE_Obj*, bool> alloc_cre_obj(size_t size, CRE_dtor_function _dtor, uint16_t t_id, AllocBuffer* buffer=nullptr);
void CRE_Obj_dtor(const CRE_Obj* x);

} // NAMESPACE_END(cre)


