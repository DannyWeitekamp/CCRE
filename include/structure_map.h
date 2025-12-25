#pragma once
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <Eigen/src/Core/NumTraits.h>
// #include <Eigen/Tensor>
#include <unsupported/Eigen/CXX11/Tensor>
#include "../include/logic.h"
#include "../include/var_inds.h" // for VarInds

// template<typename T>
// using RMatrix = Eigen::Matrix<T, Eigen::RowMajor>;

// template<typename T, typename D>
// using RTensor = Eigen::Tensor<T, D, Eigen::RowMajor>;

namespace cre {

struct SM_Score{
    SM_Score(float ub_score=0.0, float beta_score=0.0):
        ub_score(ub_score), beta_score(beta_score)
    {};

    float ub_score;
    float beta_score;

    inline cre::SM_Score operator+(const cre::SM_Score& other) const{
        return cre::SM_Score(ub_score + other.ub_score, beta_score + other.beta_score);
    }
};



// Overload operator<< for SM_Score
std::ostream& operator<<(std::ostream& os, const SM_Score& obj);

using ScoreMatrixType = Eigen::Tensor<SM_Score, 2, Eigen::RowMajor>;
using ScoreRowType = Eigen::Tensor<SM_Score, 1, Eigen::RowMajor>;


struct SM_StackItem{
    size_t i;
    size_t c;
    std::vector<int16_t> js;
    std::vector<int16_t> fixed_inds;
    float score_bound;
};


struct SM_Result{
    float score;
    std::vector<int16_t> alignment;
    std::vector<int16_t> best_remap;
    std::vector<std::tuple<float, std::vector<int16_t>>> remaps;
    std::vector<std::vector<uint8_t>> keep_masks_a = {};
    std::vector<std::vector<uint8_t>> keep_masks_b = {};

    SM_Result(float score, 
              const std::vector<int16_t>& alignment,
              const std::vector<int16_t>& best_remap,
              const std::vector<std::tuple<float, std::vector<int16_t>>>& remaps):
        score(score), alignment(alignment), best_remap(best_remap), remaps(remaps)
    {};
    // auto keep_mask_a;
    // auto keep_mask_b;
    
};

// A single candidate pair of things with variables that can be mapped to each 
//  other. Typically represent one pair of literals appearing in conjuncts of 
//  two logical statement.
struct SM_MappablePair {
    int16_t index_a;
    int16_t index_b;
    float weight;
    // std::vector<uint16_t> lit_inds_a;
    // std::vector<uint16_t> lit_inds_b;
    VarInds var_inds_a;
    VarInds var_inds_b;
    
    uint16_t n_vars;


    SM_MappablePair(float weight, 
                   int16_t index_a, int16_t index_b, 
                   uint16_t n_vars, 
                   const VarInds& var_inds_a, 
                   const VarInds& var_inds_b):
                   weight(weight), 
                   index_a(index_a), index_b(index_b), 
                   n_vars(n_vars),
                   var_inds_a(VarInds(var_inds_a)),
                   var_inds_b(VarInds(var_inds_b))
    {};

    SM_MappablePair(float weight, 
        int16_t index_a, int16_t index_b):
        weight(weight), 
        index_a(index_a), index_b(index_b), 
        n_vars(0),
        var_inds_a(VarInds()),
        var_inds_b(VarInds())
{};

    // ~SM_MappablePair(){
    //     if(n_vars > N_ITERN_INDS){
    //         delete var_inds_a.inds_ptr;
    //         delete var_inds_b.inds_ptr;
    //     }
    // };
};

// A single candidate pair of groups.
//  Typically represent a cadidate pair of conjuncts in one 
//  logical statement mapped to another.
struct SM_ConjPair {
    uint16_t conj_ind_a;
    uint16_t conj_ind_b;
    uint16_t conj_len_a;
    uint16_t conj_len_b;
    std::vector<SM_MappablePair> pairs;

    SM_ConjPair(uint16_t conj_ind_a, uint16_t conj_ind_b,
                 uint16_t conj_len_a, uint16_t conj_len_b,
                 const std::vector<SM_MappablePair>& pairs):
        conj_ind_a(conj_ind_a), conj_ind_b(conj_ind_b),
        conj_len_a(conj_len_a), conj_len_b(conj_len_b),
        pairs(pairs)
    {};
};

using VarPairWeightsType = Eigen::Tensor<float, 2, Eigen::RowMajor>;

struct SM_MapCandSet{
    size_t N; // The number of variables in l_a
    size_t M; // The number of variables in l_b
    std::vector<Item> items_a; // The conjunct-like items in l_a
    std::vector<Item> items_b; // The conjunct-like items in l_b
    // The structure weights of mapping variable i in l_a to variable j in l_b.
    //   negative weights are used to mask out invalid variable mappings.
    VarPairWeightsType var_pair_weights; 
    std::vector<SM_ConjPair> conj_pairs; // The conjunct pairs
    

    SM_MapCandSet(size_t N, size_t M,
                const std::vector<Item>& items_a,
                const std::vector<Item>& items_b,
                const VarPairWeightsType& var_pair_weights,
                const std::vector<SM_ConjPair>& conj_pairs):
        N(N), M(M), items_a(items_a), items_b(items_b),  
        var_pair_weights(var_pair_weights), conj_pairs(conj_pairs)
    {};
};



SM_MapCandSet logic_to_map_cands(ref<Logic> l_a, ref<Logic> l_b);

SM_Result structure_map_logic(
    ref<Logic> l_a, ref<Logic> l_b, 
    std::vector<int16_t>* a_fixed_inds = nullptr,
    bool drop_unconstr = false, bool drop_no_beta = false);

constexpr uint8_t AU_NORMALIZE_NONE = 0;
constexpr uint8_t AU_NORMALIZE_LEFT = 1;
constexpr uint8_t AU_NORMALIZE_RIGHT = 2;
constexpr uint8_t AU_NORMALIZE_MAX = 3;

ref<Logic> antiunify_logic(ref<Logic> l_a, ref<Logic> l_b, 
    std::vector<int16_t>* a_fixed_inds = nullptr,
    float* return_score = nullptr,
    uint8_t normalize_kind = AU_NORMALIZE_LEFT, 
    bool drop_unconstr = false, bool drop_no_beta = false);
} // namespace cre


// Define NumTraits for SM_Score Necessary to use Eigen::Tensor with SM_Score
//  and print it properly.
namespace Eigen {
    template <> struct NumTraits<cre::SM_Score> 
        : NumTraits<float> // Inherit most traits from float
    {
        // Override specific traits as needed
        typedef cre::SM_Score NonInteger;
        typedef cre::SM_Score Nested;
        typedef float Real; // This must be a standard type std::log2 can use
        typedef float Literal;
        enum {
            IsComplex = 0,
            IsInteger = 0,
            IsSigned = 1,
            RequireInitialization = 1,
            ReadCost = 1,
            AddCost = 3,
            MulCost = 3
        };
    };

    
} // namespace Eigen
