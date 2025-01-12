#pragma once

#include "../include/item.h"
#include "../include/hash.h"

namespace cre {

struct Member : public Item {
    uint64_t hash;

    Member() : Item(), hash(0)
    {};

    template<class T>
    Member(T val, uint64_t _hash=0) :
            Item(val), hash(_hash)
    {
        if(_hash == 0){
            hash = CREHash{}(val); 
        }
    }
};

} // NAMESPACE_END(cre)
