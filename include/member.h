#pragma once

#include "../include/item.h"
#include "../include/hash.h"

namespace cre {


// struct Member;

// template<typename T>
// concept NotMember = !std::is_same_v<T, Member>;

struct Member : public Item {
    uint64_t hash;

    Member() : Item(), hash(0)
    {};

    template<typename T>
    inline uint64_t ensure_hash(T&& value, uint64_t _hash=0){
        // cout << "MEMBER :" << value;
        if(_hash == 0){
            _hash = CREHash{}(value); 
        }
        return _hash;
    }

    // template<typename T>
    // // requires NotMember<T>
    // Member(const T& value, uint64_t _hash=0) :
    //         hash(ensure_hash(value, _hash)), Item(value) 
    // {
    //     cout << " COPY" << endl;
    // }

    template<typename T>
    // requires NotMember<T>
    Member(T&& value, uint64_t _hash=0) :
            hash(ensure_hash(value, _hash)), 
            Item(std::forward<T>(value))
    {
        // cout << " MOVE" << endl;
    }

    Member(const Member& other) :
        Item(other), hash(other.hash)
    {};

    Member(const Member& other, uint8_t val_kind, [[maybe_unused]] bool _) :
        Item(other, val_kind), hash(other.hash)
    {};

    Member& operator=(const Member&) = default;
    Member& operator=(Member&&) = default;
    Member(Member&&) = default;

    inline Member to_weak(){
        // cout << "AAAH" << this->get_wrefcount() << endl;
        // Member m = Member(*this);
        // m.val_kind = WEAK_REF;
        // return m;
        return Member(*this, WEAK_REF, true);
    }

    inline Item to_strong(){
        return Member(*this, STRONG_REF, true);        
        // Member m = Member(*this);
        // m.val_kind = STRONG_REF;
        // return m;
    }

    inline Item to_raw_ptr(){
        return Member(*this, RAW_PTR, true);
        // Member m = Member(*this);
        // m.val_kind = RAW_PTR;
        // return m;
    }

    
    using Item::operator==; 
};

} // NAMESPACE_END(cre)
