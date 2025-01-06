#ifndef _MEMBER_H_
#define _MEMBER_H_

#include "../include/item.h"
#include "../include/hash.h"

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


#endif // _MEMBER_H_
