#ifndef _CRE_ALLOC_BUFFER_H_
#define _CRE_ALLOC_BUFFER_H_

#include <stdint.h>
#include <stddef.h>
#include "cre_obj.h"
// #include "alloc_buffer.h"

//--------------------------------------------------------------
// : AllocBuffer

struct AllocBuffer : public CRE_Obj {
    // --- Members ---
    uint8_t* data;
    uint8_t* head;
    uint8_t* end;
    size_t size;
    size_t alignment;
    bool resizeable;
    
    // --- Methods ---
    AllocBuffer(size_t n_bytes, bool resizeable=false);
    ~AllocBuffer();
    // void add_data(uint8_t* data, size_t size);

    uint8_t* resize(size_t n_bytes);
    uint8_t* alloc_bytes(size_t n_bytes, bool& did_malloc);
    uint8_t* alloc_bytes(size_t n_bytes);
    uint8_t* free_back(size_t n_bytes);
};

#endif /* _CRE_ALLOC_BUFFER_H_ */
