#include <atomic>
#include <iostream>
#include "../include/cre_obj.h"

using namespace std;

CRE_Obj::CRE_Obj(CRE_dtor_function _dtor){
	#ifndef CRE_NONATOMIC_REFCOUNT
		atomic_init(&this->ref_count, 1);
		// this->ref_count.store(1, std::memory_order_relaxed);
	#else
		this->ref_count = 1;
	#endif
	// this->ref_count = 1;
	this->dtor = _dtor;
}

CRE_Obj::~CRE_Obj(){
	if(dtor){
		this->dtor((void*) this);
	}
}

size_t CRE_Obj::get_refcount(){
	return this->ref_count;
}

extern "C" size_t CRE_get_refcount(CRE_Obj* x){
	return x->ref_count;
}

extern "C" void CRE_incref(CRE_Obj* x){
	// cout << "INCREF" << endl;
	#ifndef CRE_NONATOMIC_REFCOUNT
		x->ref_count.fetch_add(1, std::memory_order_relaxed);
	#else
   		x->ref_count++; 
	#endif
}

extern "C" void CRE_decref(CRE_Obj* x){
	// cout << "DECREF" << endl;

	#ifndef CRE_NONATOMIC_REFCOUNT
		x->ref_count.fetch_sub(1, std::memory_order_relaxed);
	#else
   		x->ref_count--;
	#endif
	
	
	if (x->ref_count == 0) {
		if (x->dtor){
			x->dtor(x);
		}
		// cout << "DELETE" << endl;
		delete x;
    }
}


