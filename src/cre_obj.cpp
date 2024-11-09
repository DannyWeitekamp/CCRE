#include <atomic>
#include <iostream>
#include "../include/cre_obj.h"


using std::cout;
using std::endl;

CRE_Obj::CRE_Obj(){//(CRE_dtor_function _dtor){
	#ifndef CRE_NONATOMIC_REFCOUNT
		atomic_init(&this->ref_count, 0);
		// this->ref_count.store(1, std::memory_order_relaxed);
	#else
		this->ref_count = 0;
	#endif
	// this->ref_count = 1;
	// this->dtor = _dtor;
	this->alloc_buffer = (AllocBuffer*) nullptr;
}

int64_t CRE_Obj::get_refcount() noexcept{
	return ref_count;
}

void CRE_Obj::operator delete(void* p){
	CRE_Obj* x = (CRE_Obj*) p;
	cout << "DELETE:" << x->alloc_buffer << endl;
	if(x->alloc_buffer == nullptr){
    	delete p;
	}else{
		// NOTE: We need to do this because cannot
		//  write alloc_buffer as a ref<AllocBuffer> 
		CRE_decref((CRE_Obj*) x->alloc_buffer);
	}
}

// extern "C" int64_t CRE_get_refcount(const CRE_Obj* x){
// 	return x->ref_count;
// }

// extern "C" void CRE_incref(const CRE_Obj* x){
// 	// cout << "INCREF" << endl;
// 	#ifndef CRE_NONATOMIC_REFCOUNT
// 		x->ref_count.fetch_add(1, std::memory_order_relaxed);
// 	#else
//    		x->ref_count++; 
// 	#endif
// }

// extern "C" void CRE_addref(const CRE_Obj* x, size_t n){
// 	// cout << "INCREF" << endl;
// 	#ifndef CRE_NONATOMIC_REFCOUNT
// 		x->ref_count.fetch_add(n, std::memory_order_relaxed);
// 	#else
//    		x->ref_count += n; 
// 	#endif
// }

// inline void _check_destroy(const CRE_Obj* x){
// 	cout << "REF CNT:" << x->ref_count << endl;
// 	if (x->ref_count <= 0) {
// 		// if (x->dtor){
// 		// 	x->dtor(x);
// 		// }
		
// 		// if(x->alloc_buffer != nullptr){
// 		// 	CRE_decref((CRE_Obj*) x->alloc_buffer);
// 		// }else{
// 		delete x;
// 			// free(x);
// 		// }
//     }
// }

// extern "C" void CRE_decref(const CRE_Obj* x){
// 	// cout << "DECREF" << endl;

// 	#ifndef CRE_NONATOMIC_REFCOUNT
// 		x->ref_count.fetch_sub(1, std::memory_order_relaxed);
// 	#else
//    		x->ref_count--;
// 	#endif

// 	_check_destroy(x);
// }

// extern "C" void CRE_subref(const CRE_Obj* x, size_t n){
// 	// cout << "INCREF" << endl;
// 	#ifndef CRE_NONATOMIC_REFCOUNT
// 		x->ref_count.fetch_sub(n, std::memory_order_relaxed);
// 	#else
//    		x->ref_count -= n; 
// 	#endif

//    	_check_destroy(x);
// }


