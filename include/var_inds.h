#pragma once

#include <cstddef>
#include <cstdint>
#include <stddef.h>              // for size_t, NULL
#include <stdint.h>              // for uint16_t, int16_t, uint32_t, uint64_t
#include <iosfwd>                // for ostream
#include <ostream>               // for operator<<
#include <string>                // for string
#include <string_view>           // for string_view
#include <vector>                // for vector


namespace cre {

constexpr uint16_t N_ITERN_VAR_INDS = 7;

struct VarInds {
    uint16_t inds[N_ITERN_VAR_INDS];
    uint16_t _size;
    uint16_t* inds_ptr;
    
    uint16_t size() const{
        return _size;
    }

    VarInds(const VarInds& other): _size(other._size){
        if(other.size() <= N_ITERN_VAR_INDS){
            std::copy(other.inds, other.inds + other._size, inds);
        }else{
            inds_ptr = (uint16_t*) malloc(other.size() * sizeof(uint16_t));
            std::copy(other.inds_ptr, other.inds_ptr + other.size(), inds_ptr);
        }
    }

    template <std::integral T>
    VarInds(const std::vector<T>& other) : _size(other.size()){
        if(other.size() <= N_ITERN_VAR_INDS){
            inds_ptr = &inds[0];
        }else{
            inds_ptr = (uint16_t*) malloc(other.size() * sizeof(uint16_t));
            std::copy(other.begin(), other.end(), inds_ptr);
            // inds_ptr = new std::vector<uint16_t>(other.begin(), other.end());
        }
    }

    ~VarInds(){
        if(_size > N_ITERN_VAR_INDS && inds_ptr != nullptr){
            free(inds_ptr);
        }
    }
    
    void reserve(size_t __size){
        if(_size > N_ITERN_VAR_INDS &&  inds_ptr != nullptr){
            free(inds_ptr);
        }
        if(__size > N_ITERN_VAR_INDS){
            inds_ptr = (uint16_t*) malloc(_size * sizeof(uint16_t));
        }
        _size = __size;
    }
    
    uint16_t operator[](size_t i) const {
        // NEEDS BENCMARKING: is it better to eat the branch or not?
        return _size <= N_ITERN_VAR_INDS ? inds[i] : inds_ptr[i];
    }

    uint16_t& operator[](size_t i) {
        return _size <= N_ITERN_VAR_INDS ? inds[i] : inds_ptr[i];
    }

    VarInds(): _size(0) {}
    // VarInds(size_t size): _size(size) {}
};


inline std::ostream& operator<<(std::ostream& out, const VarInds& v) {
    out << '[';
    for (uint16_t i = 0; i < v.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << v[i];
    }
    out << ']';
    return out;
}



} // namespace cre