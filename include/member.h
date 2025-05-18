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

    Member(const Member& other) :
        Item(other), hash(other.hash)
    {cout << "I HOPE NOT THIS" << endl;};

    Member(const Member& other, uint8_t val_kind) :
        Item(other, val_kind), hash(other.hash)
    {cout << "I HOPE THIS" << endl;};

    Member& operator=(const Member&) = default;
    Member& operator=(Member&&) = default;
    Member(Member&&) = default;

    inline Member to_weak(){
        cout << "AAAH" << this->get_wrefcount() << endl;
        return Member(*this, WEAK_REF);
    }

    inline Item to_strong(){
        return Member(*this, STRONG_REF);        
    }

    inline Item to_raw_ptr(){
        return Member(*this, RAW_PTR);
    }
};

} // NAMESPACE_END(cre)
