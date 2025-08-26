#include <atomic>
#include <iostream>

#include "../include/context.h"
#include "../include/cre_obj.h"
#include "../include/alloc_buffer.h"


using std::cout;
using std::endl;

namespace cre {


ControlBlock::ControlBlock(CRE_Obj* _obj_ptr, CRE_dtor_function _dtor, uint16_t _t_id) : 
	obj_ptr(_obj_ptr), dtor(_dtor), t_id(_t_id)
{
	// cout << "init CB: " << uint64_t(this) << endl;
}

ControlBlock::~ControlBlock(){
	// cout << "destroy CB: " << uint64_t(this) << endl;
	global_cb_pool.dealloc((ControlBlock*) this);
}

// ControlBlock* ControlBlockPool::alloc_block(){
// 	ControlBlock* block_begin = reinterpret_cast<ControlBlock*>(malloc(block_size));
// 	ControlBlock* chunk = block_begin;
// 	for (int i = 0; i < chunks_per_block - 1; ++i) {
// 	    chunk->next = 
// 	        reinterpret_cast<ControlBlock*>(reinterpret_cast<char *>(chunk) + sizeof(ControlBlock));
// 	    chunk = chunk->next;
//   	}
//   	chunk->next = nullptr;
//   	return block_begin;
// }

// ControlBlock* ControlBlockPool::alloc(){
// 	if (alloc_ptr == nullptr) {
//     	alloc_ptr = alloc_block();
//   	}
//   	ControlBlock* free_chunck = alloc_ptr;
//   	alloc_ptr = alloc_ptr->next;
//   	return free_chunck;
// }

// void ControlBlockPool::dealloc(ControlBlock* ptr){
// 	reinterpret_cast<ControlBlock*>(ptr)->next = alloc_ptr;
// 	alloc_ptr = reinterpret_cast<ControlBlock*>(ptr);
// }


// CRE_Obj::init(CRE_dtor_function _dtor){

// 	this->control_block = new ControlBlock(this, _dtor);
// 	// (ControlBlock*) malloc(sizeof(this->control_block));

// 	#ifndef CRE_NONATOMIC_REFCOUNT
// 		atomic_init(&this->ref_count, 0);
// 		// this->ref_count.store(1, std::memory_order_relaxed);
// 	#else
// 		this->ref_count = 0;
// 	#endif


// 	// this->ref_count = 1;
// 	// this->dtor = _dtor;
// 	// this->alloc_buffer = (AllocBuffer*) nullptr;
// }

int64_t CRE_Obj::get_refcount() noexcept{
	return ref_count;
}

int64_t CRE_Obj::get_wrefcount() noexcept{
	return this->control_block->get_wrefcount();
}



void CRE_Obj::init_control_block(CRE_dtor_function _dtor, uint16_t t_id){
	// this->control_block = new ControlBlock(this, _dtor);

	
    ControlBlock* data = global_cb_pool.alloc();
    // cout << "INIT CONTROL BLOCK: " << uint64_t(data) << ", " << uint64_t(this) << endl;
    this->control_block = new (data) ControlBlock(this, _dtor, t_id);

    // cout << "INIT: " << get_wrefcount(); 


}

std::tuple<CRE_Obj*, bool> alloc_cre_obj(size_t size, CRE_dtor_function _dtor, uint16_t t_id, AllocBuffer* buffer){
    bool did_malloc = false;
    CRE_Obj* obj = nullptr;
    if(buffer != nullptr){
        obj = (CRE_Obj*) buffer->alloc_bytes(size, did_malloc);
    }else{
        did_malloc = true;
        obj = (CRE_Obj*) malloc(size);
    }
    obj->init_control_block(_dtor, t_id);

    if(!did_malloc){
        obj->control_block->alloc_buffer = buffer;
        buffer->inc_ref();
    }

    return {obj, did_malloc};
}

void CRE_Obj_dtor(const CRE_Obj* x){
    if(x->control_block->alloc_buffer == nullptr){
        free((void*) x);
    }else{
        // NOTE: We need to do this because cannot
        //  write alloc_buffer as a ref<AllocBuffer> 
        // cout << "alloc buff refcount=" << x->alloc_buffer->get_refcount() << endl;
        x->control_block->alloc_buffer->dec_ref();
    }
}


// void CRE_Obj::operator delete(void* p){
// 	CRE_Obj* x = (CRE_Obj*) p;
// 	// cout << "DELETE:" << x->alloc_buffer << endl;
// 	if(x->alloc_buffer == nullptr){
//     	x->dtor(x);
// 	}else{
// 		// NOTE: We need to do this because cannot
// 		//  write alloc_buffer as a ref<AllocBuffer> 
// 		((CRE_Obj*) x->alloc_buffer)->dec_ref();
// 	}
// }

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

} // NAMESPACE_END(cre)
