#include <cstddef>
#include <cstdint>
#include <vector>
#include <Eigen/Dense>
// #include <Eigen/Tensor>
#include <unsupported/Eigen/CXX11/Tensor>
#include "../include/logic.h"

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
};

typedef Eigen::Tensor<SM_Score, 2, Eigen::RowMajor> ScoreMatrixType;


struct SM_StackItem{
    size_t i;
    size_t c;
    std::vector<int16_t> js;
    std::vector<int16_t> fixed_inds;
};


struct SM_Result{
    float score;
    std::vector<int16_t> alignment;
    std::vector<int16_t> remap;
    // auto keep_mask_a;
    // auto keep_mask_b;
    
};

static constexpr size_t N_ITERN_INDS = 4;

struct ItemVarInds {
    union{
        uint16_t inds[N_ITERN_INDS];
        std::vector<uint16_t>* inds_ptr; 
    };

    ItemVarInds(const std::vector<uint16_t>& inds_v){
        if(inds_v.size() <= 4){
            std::copy(inds_v.begin(), inds_v.end(), inds);
        }else{
            inds_ptr = new std::vector<uint16_t>(inds_v.begin(), inds_v.end());

        }
    };

    ItemVarInds(): inds_ptr(nullptr) {};
};

// A single candidate pair of things with variables that can be mapped to each 
//  other. Typically represent one pair of literals appearing in conjuncts of 
//  two logical statement.
struct SM_MappablePair {
    int16_t index_a;
    int16_t index_b;
    // std::vector<uint16_t> lit_inds_a;
    // std::vector<uint16_t> lit_inds_b;
    ItemVarInds var_inds_a;
    ItemVarInds var_inds_b;
    float weight;
    uint16_t n_vars;


    SM_MappablePair(float weight, 
                   int16_t index_a, int16_t index_b, 
                   uint16_t n_vars, 
                   const std::vector<uint16_t>& var_inds_a, 
                   const std::vector<uint16_t>& var_inds_b):
                   weight(weight), 
                   index_a(index_a), index_b(index_b), 
                   n_vars(n_vars),
                   var_inds_a(ItemVarInds(var_inds_a)),
                   var_inds_b(ItemVarInds(var_inds_b))
    {};

    SM_MappablePair(float weight, 
        int16_t index_a, int16_t index_b):
        weight(weight), 
        index_a(index_a), index_b(index_b), 
        n_vars(0),
        var_inds_a(ItemVarInds()),
        var_inds_b(ItemVarInds())
{};

    ~SM_MappablePair(){
        if(n_vars > N_ITERN_INDS){
            delete var_inds_a.inds_ptr;
            delete var_inds_b.inds_ptr;
        }
    };
};

// A single candidate pair of groups.
//  Typically represent a cadidate pair of conjuncts in one 
//  logical statement mapped to another.
struct SM_GroupPair {
    uint16_t conj_ind_a;
    uint16_t conj_ind_b;
    std::vector<SM_MappablePair> pairs;

    SM_GroupPair(uint16_t conj_ind_a, uint16_t conj_ind_b, std::vector<SM_MappablePair>& pairs):
        conj_ind_a(conj_ind_a), conj_ind_b(conj_ind_b), pairs(pairs)
    {};
};



std::vector<SM_GroupPair> make_group_pairs(ref<Logic> l_a, ref<Logic> l_b);


} // namespace cre