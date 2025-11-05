#include <stdint.h>        // for uint8_t
#include <stdlib.h>        // for malloc
#include <cstring>         // for size_t, memcpy
#include <stdexcept>       // for runtime_error
#include <string>          // for operator+, to_string
#include "alloc_buffer.h"  // for AllocBuffer
namespace cre { class CRE_Obj; }


namespace cre {

//--------------------------------------------------------------
// : AllocBuffer

void AllocBuffer_dtor(const CRE_Obj* x){
	// std::cout << "Free AllocBuffer" << std::endl;
	AllocBuffer* self = ((AllocBuffer* ) x);
	delete self;
}

AllocBuffer::AllocBuffer(size_t n_bytes, bool _resizeable){
	this->init_control_block(&AllocBuffer_dtor);
	
	// NOTE: Might play with explicit alignment requirements later
	// cout << "Buffer size: " << n_bytes << " " << endl; 
	uint8_t* data = new uint8_t[n_bytes];//(uint8_t*) malloc(n_bytes);
	// std::cout << "DPTR: " << uint64_t(data) << std::endl;
	// std::cout << "NBYTES: " << n_bytes << std::endl;
	// if(ptr == NULL){
	// 	stringstream err;
	// 	err << "fatal: out of memory (malloc(" << n_bytes << ")).";
	// 	throw std::runtime_error(err.str());
	// }
	this->data = data;
	this->head = data;
	this->end =  data+n_bytes;
	this->size =  n_bytes;
	this->resizeable = _resizeable;
	// this->size = size;
};

AllocBuffer::~AllocBuffer(){
	delete[] data;
}

uint8_t* AllocBuffer::resize(size_t n_bytes){
	if(!this->resizeable){
		throw std::runtime_error("Attempt to resize unresizeable AllocBuffer.");
	}
	uint8_t* old_data = this->data;
	this->data = new uint8_t[n_bytes];
	std::memcpy(this->data, old_data, n_bytes);
	this->end = this->data+n_bytes;
	this->size = n_bytes;
	return this->data;
}

uint8_t* AllocBuffer::alloc_bytes(size_t n_bytes, bool& did_malloc){
	// return (uint8_t*) malloc(n_bytes);
	if(this->head + n_bytes > this->end){
		// if(this->resizeable){
		// 	std::cout << "RESIZE: " << this->resizeable << std::endl;
		// 	// Grow by at least n_bytes. By default double the size, 
		// 	//  but don't grow more than a page ~4kb
		// 	size_t min_grow = std::min(size_t(4096), this->size);
		// 	this->resize(this->size + std::max(min_grow, n_bytes));
		// }else{
			// std::cout << "DID MALLOC(" << n_bytes << ")" << std::endl;
			// If the buffer is not allowed to resize, (which is the default since
			//   in most cases resizing would cause pointer invalidations),
			//   then fall back on malloc().
			did_malloc = true;
			return (uint8_t*) malloc(n_bytes);
		// }
	}
	// std::cout << "FROM HEAD: " << uint64_t(this->head) << std::endl;
	did_malloc = false;
	uint8_t* data = this->head;
	this->head += n_bytes;
	return data;
}

uint8_t* AllocBuffer::alloc_bytes(size_t n_bytes){
	bool did_malloc;
	return this->alloc_bytes(n_bytes, did_malloc);
}

uint8_t* AllocBuffer::free_back(size_t n_bytes){
	throw std::runtime_error("not implemented. n_bytes=" + std::to_string(n_bytes));
	return 0;
}

// AllocBuffer::insert_fact_slice(Fact* fact, int start, int end){
// 	start = start >=0 ? start : start->length + start;
// 	end = end >=0 ? end : fact->length + end;
// 	size_t length = end-start;
// 	size_t fact_n_bytes = SIZEOF_FACT(length);
// 	if(this->head+fact_n_bytes >= this->end){
// 		// Don't grow more than a page (usually 4k bytes)
// 		this->resize(this->size + std::min(this->size, 4096));
// 	}
// 	Fact* new_fact = (Fact *) this->head;

// 	// TODO Copy Fact;
	
// 	_init_fact(new_fact, length, fact->type);
// 	memcpy(this->head, uin ,length*sizeof(Item));
	
// 	// memcpy(this->head, fact, fact_n_bytes);
// 	this->head += fact_n_bytes
// }

} // NAMESPACE_END(cre)
