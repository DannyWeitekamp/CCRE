#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cmath>
#include "../include/item.h"
#include "../include/logic.h"
#include "../include/structure_map.h"
#include "../include/var_inds.h" // for VarInds
#include "types.h"
#include <sys/types.h>
#include <vector>
#include <unsupported/Eigen/CXX11/Tensor>
#include <Eigen/Dense>
#include <algorithm>
#include <assert.h>




namespace cre {


// :--------------------------------------------------:
// :  Various Helper Functions for Structure Mapping 
// :--------------------------------------------------:

struct FunctionallyEqual {
    bool operator()(const Item& a, const Item& b) const {
        cout << "FunctionallyEqual: " << a << " , " << b << " Result: " << items_equal(a, b, false, true) << endl;
        return items_equal(a, b, false, true);
    }
};    
using ItemMapType = ankerl::unordered_dense::map<Item, std::vector<std::tuple<int16_t, Item>>, CREHash, FunctionallyEqual> ;

bool all_fixed(std::vector<int16_t>& fixed_inds){
    return std::all_of(fixed_inds.begin(), fixed_inds.end(), [](int16_t x){ return x != -2; });
}

// Function to get the argsort of an Eigen VectorXf
std::vector<int16_t> descending_argsort(const Eigen::TensorRef<ScoreRowType>& v) {
    // Initialize a vector of indices from 0 to v.size() - 1
    std::vector<int16_t> indices(v.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on the values in v
    std::stable_sort(indices.begin(), indices.end(),
                     [&v](int i1, int i2) { return v(i1).ub_score >= v(i2).ub_score; });

    return indices;
}


// ---- GENERTIC STRUCTURE MAPPING IMPLEMENTATION ----

// :--------------------------------------------------:
// :  _calc_remap_score_matrices() : (private - helps structure_map_generic())
// :  Calculates the upper bound on the matching score for a partial mapping
// :--------------------------------------------------:


void _calc_remap_score_matrices(
    std::vector<SM_ConjPair>& conj_pairs,
    std::vector<ScoreMatrixType>& score_matrices, 
    std::vector<int16_t>& a_fixed_inds){

    assert(score_matrices.size() > 0);

    size_t M = score_matrices[0].dimension(1);

    auto b_fixed_inds = std::vector<int16_t>(M, -2);
    for(size_t i = 0; i < a_fixed_inds.size(); i++){
        int16_t a_ind = a_fixed_inds[i];
        if(a_ind != -2){
            b_fixed_inds[a_ind] = i;
        }
    }

    for(size_t i = 0; i < conj_pairs.size(); i++){
        auto& cp = conj_pairs[i];
        ScoreMatrixType& score_matrix = score_matrices[i];
        
        // Reset Score Matrix
        score_matrix.setConstant(SM_Score(0.0,0.0));

    // for(size_t i = 0; i < groups_ij.size(); i++){
        for(SM_MappablePair& mp : cp.pairs){
            switch(mp.n_vars){
                case 1:
                {
                    int16_t ind_a0 = mp.var_inds_a.inds[0];
                    int16_t ind_b0 = mp.var_inds_b.inds[0];
                    int16_t fix_b0 = a_fixed_inds[ind_a0];
                    int16_t fix_a0 = b_fixed_inds[ind_b0];
                    if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                       (fix_a0 == -2 or fix_a0 == ind_a0)){
                        score_matrix(ind_a0, ind_b0).ub_score += mp.weight;
                    }
                    break;
                }
                case 2:
                {
                    
                    int16_t ind_a0 = mp.var_inds_a.inds[0];
                    int16_t ind_b0 = mp.var_inds_b.inds[0];
                    int16_t ind_a1 = mp.var_inds_a.inds[1];
                    int16_t ind_b1 = mp.var_inds_b.inds[1];
                    int16_t fix_b0 = a_fixed_inds[ind_a0];
                    int16_t fix_a0 = b_fixed_inds[ind_b0];
                    int16_t fix_b1 = a_fixed_inds[ind_a1];
                    int16_t fix_a1 = b_fixed_inds[ind_b1];
                    if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                       (fix_a0 == -2 or fix_a0 == ind_a0) &&
                       (fix_b1 == -2 or fix_b1 == ind_b1) &&
                       (fix_a1 == -2 or fix_a1 == ind_a1) ){
                        score_matrix(ind_a0, ind_b0).ub_score += mp.weight;
                        score_matrix(ind_a1, ind_b1).ub_score += mp.weight;
                        score_matrix(ind_a0, ind_b0).beta_score += mp.weight;
                        score_matrix(ind_a1, ind_b1).beta_score += mp.weight;
                    }
                    break;
                }
                default:
                    throw std::runtime_error("NOT IMPLEMENTED");

            }
        }
    }
    return;
}

// :--------------------------------------------------:
// :  align_greedy() : (private - helps _get_best_alignment())
// :  Greedily aligns conjuncts in A with conjuncts in B based on the 
// :  upper bound on the matching score.
// :--------------------------------------------------:

std::vector<int16_t> align_greedy(ScoreMatrixType& score_matrix){
    size_t N = score_matrix.dimension(0);
    size_t M = score_matrix.dimension(1);
    auto alignment = std::vector<int16_t>(N, -1);
    auto covered = std::vector<bool>(M, false);
    
    size_t c = 0;
    // Initialize a vector of indices from 0 to v.size() - 1
    std::vector<int16_t> flat_inds(N*M);
    std::iota(flat_inds.begin(), flat_inds.end(), 0);
    SM_Score* data = score_matrix.data();

    // Sort the indices based on the values in v
    std::stable_sort(flat_inds.begin(), flat_inds.end(),
                     [&data](int i1, int i2) { return data[i1].ub_score >= data[i2].ub_score; });

    for(size_t flat_ind : flat_inds){
        size_t i = flat_ind / M;
        size_t j = flat_ind % M;
        SM_Score& score = data[flat_ind];
        if(score.ub_score == 0.0 or c == N){
            break;
        }
        if(alignment[i] == -1 && !covered[j]){
            alignment[i] = int16_t(j);
            covered[j] = true;
            c += 1;
        }
    }
    return alignment;
}

float score_remap(
    const std::vector<int16_t>& remap,
    ScoreMatrixType& score_matrix,
    const VarPairMatrix& var_pairs
){

    // cout << "SCORE_REMAP: " << remap << endl;
    // cout << "SCORE_MATRIX: " << score_matrix << endl;

    float score = 0.0f;
    for(size_t i = 0; i < remap.size(); i++){
        if(remap[i] < 0) continue;
        // cout << "SCORE_MATRIX(" << i << ", " << remap[i] << "): " << score_matrix(i, remap[i]) << endl;

        // The item component of score for of mapping i -> remap[i] is the upper bound 
        //   which so far has double counted betas minus half of the beta score.
        //   which has also been double counting betas (but not counting alphas)
        score += (score_matrix(i, remap[i]).ub_score 
                 -(score_matrix(i, remap[i]).beta_score / 2));
        
        // Add in the variable weight if i is mapped to a variable in B
        //   or still unfixed (i.e. -2)
        if(remap[i] != -1 && var_pairs(i, remap[i]).weight >= 0.0f){
            score += var_pairs(i, remap[i]).weight;
        } 

    }
    // cout << "SCORE: " << score << endl;
    return score;
}

// :--------------------------------------------------:
// :  _get_best_alignment() : (private - helps structure_map_generic())
// :  Finds the best alignment of conjuncts between A and B, and 
// :  the cumulative score matrix from this alignment.
// :--------------------------------------------------:

std::vector<int16_t> _get_best_alignment(
    size_t Da, size_t Db,
    std::vector<SM_ConjPair>& conj_pairs,
    std::vector<ScoreMatrixType>& score_matrices, 
    ScoreMatrixType& cum_score,
    const VarPairMatrix& var_pairs
    ){

    size_t N = cum_score.dimension(0);
    size_t M = cum_score.dimension(1);
    
    if(score_matrices.size() == 1){
        cum_score = score_matrices[0];
        return std::vector<int16_t>(1, 0);
    }

    // cout << "--- THIS CASE!! ---" << endl;

    cum_score.setConstant(SM_Score(0.0,0.0));

    auto align_matrix = ScoreMatrixType(Da, Db);
    // auto remaps = Eigen::MatrixXd<int16_t, Eigen::RowMajor>(Da, Db, N);

    for(size_t k = 0; k < conj_pairs.size(); k++){
        auto& cp = conj_pairs[k];
        auto& score_matrix = score_matrices[k];
        auto remap = align_greedy(score_matrix);
        size_t i = cp.conj_ind_a;
        size_t j = cp.conj_ind_b;
        align_matrix(i,j).ub_score = score_remap(remap, score_matrix, var_pairs);
    }

    auto alignment = align_greedy(align_matrix);

    // cout << "align matrix: " << endl << align_matrix << endl;
    // cout << "alignment: " << alignment << endl;

    // Fill in cumulative score matrix
    for(size_t k = 0; k < conj_pairs.size(); k++){
        auto& cp = conj_pairs[k];
        auto& score_matrix = score_matrices[k];
        size_t i = cp.conj_ind_a;
        size_t j = cp.conj_ind_b;
        if(alignment[i] == j){
            cum_score += score_matrix;
        }
    }

    return alignment;
}





// :--------------------------------------------------:
// :  get_best_ind_iter() : (private - helps structure_map_generic())
// :  Finds the best next iterator for variables assignments in A.
// :  High scoring assignments are iterated over first.
// :--------------------------------------------------:

std::tuple<int64_t, std::vector<int16_t>> 
  get_best_ind_iter(ScoreMatrixType& cum_score, std::vector<int16_t>& a_fixed_inds){
    std::tuple<int64_t, std::vector<int16_t>> best_iter = std::make_tuple(-1, std::vector<int16_t>());

    // Choose the best best next iterator among the still unfixed variables. Maximize firstly
    //  the maximum score that fixing the mapping for a var i would contribute, and 
    //  secondly the maximum difference between the max assignment and mean of the alternate
    //  assignments, to capture how unambiguously i's max scoring assignment is best.
    std::tuple<float, float> best_i_quality = std::make_tuple(-1, 0.0);
    for(int64_t i = 0; i < cum_score.dimension(0); i++){
        // Skip if already assigned
        if(a_fixed_inds[i] != -2){
            continue;
        }

        Eigen::TensorRef<ScoreRowType> row = cum_score.chip(i, 0); 
        auto inds = descending_argsort(row);
        float max_score = row(inds[0]).ub_score;

        //Don't make iterators for rows of all zeros
        if(max_score == 0){
            continue;
        }
        
        // Find the first index with the lowest score
        float row_min = row(inds.size()-1).ub_score;
        int16_t argmin_ind = inds.size()-1;
        for(int16_t j = inds.size()-2; j > 0; j--){
            if(row(inds[j]).ub_score <= row_min){
                argmin_ind = j;
            }else{
                break;
            }
        }

        // Initialize i_quality to the max score and the max score
        std::tuple<float, float> i_quality = std::make_tuple(max_score, max_score);

        // If there is at least one score higher than the others 
        if(argmin_ind > 1){
            // Find the mean difference between the highest score and the others
            float mean_diff = 0.0;
            for(size_t j = 1; j < argmin_ind+1; j++){
                mean_diff += max_score - row(inds[j]).ub_score;
            }
            mean_diff /= argmin_ind - 1;
            i_quality = std::make_tuple(max_score, mean_diff);
        }

        // Assign the best iterator so far.
        if(i_quality > best_i_quality){
            best_iter = std::make_tuple(i, std::move(inds));
            best_i_quality = i_quality;
        }
    }

    return best_iter;
}


struct MapProblem {
    std::vector<int16_t> alignment;
    std::vector<int16_t> fixed_inds;
    float best_score;
    float score_bound;
    std::vector<int16_t>* best_remap;
    std::vector<std::tuple<float, std::vector<int16_t>>> remaps;
    std::vector<SM_StackItem> iter_stack;
    ScoreMatrixType cum_score;
    std::vector<ScoreMatrixType> score_matrices;
    VarPairMatrix var_pairs;
    float tolerance;
    bool drop_unconstr;
    bool drop_no_beta;

    MapProblem(size_t Da, size_t N, size_t M, size_t tot_n_pairs,
               std::vector<int16_t>* a_fixed_inds, 
               const VarPairMatrix& var_pairs,
               float tolerance, bool drop_unconstr, bool drop_no_beta):
        alignment(Da, -2),
        fixed_inds(N, -2),
        best_score(0.0f),
        score_bound(0.0f),
        best_remap(nullptr),
        remaps({}),
        iter_stack({}),
        cum_score(N, M),
        score_matrices(tot_n_pairs, ScoreMatrixType(N, M)),
        var_pairs(var_pairs),
        tolerance(tolerance),
        drop_unconstr(drop_unconstr),
        drop_no_beta(drop_no_beta)
    {
        if(a_fixed_inds != nullptr){
            for(size_t i = 0; i < a_fixed_inds->size(); i++){
                fixed_inds[(*a_fixed_inds)[i]] = i;
            }
        }
    };

    // :--------------------------------------------------:
// :  fill_unambiguous_inds() : (private - helps structure_map_generic())
// :  Finds the best unambiguous mapping for each variable in A.
// :  This is done by greedily assigning each variable to the
// :  variable in B with the highest score, and then dropping
// :  variables that have no assignments with a non-zero score.
// :  If drop_no_beta=True, then variables with no beta literals
// :  that support any assignment to the variable are also dropped.
// :  If drop_unconstr=True, then variables with no assignments
// :  with a non-zero score are also dropped.
// :--------------------------------------------------:
    uint16_t fill_unambiguous_inds(){
        size_t N = cum_score.dimension(0);
        size_t M = cum_score.dimension(1);
        auto& unamb_inds = fixed_inds;
        // std::vector<int16_t> unamb_inds = std::vector<int16_t>(
        //     fixed_inds.begin(), fixed_inds.end()); // copy of a_fixed_inds
        auto unconstr_mask = std::vector<uint8_t>(fixed_inds.size());
        uint16_t new_unamb = 0;
        

        for(size_t a_ind = 0; a_ind < N; a_ind++){
            // Don't touch if already assigned  
            if(fixed_inds[a_ind] != -2){
                continue;
            }

            // Find any assignments with non-zero score
            uint16_t cnt = 0;
            uint16_t beta_cnt = 0;
            uint16_t nz_b_ind = -1;
            for(size_t b_ind = 0; b_ind < M; b_ind++){
                if(cum_score(a_ind, b_ind).ub_score != 0){
                    cnt++;
                    nz_b_ind = b_ind;
                }
                if(cum_score(a_ind, b_ind).beta_score != 0){
                    beta_cnt++;
                }
            }

            // IF drop_no_beta=True and no beta literals 
            //  support any assignment to a_ind and then drop.
            if(drop_no_beta && beta_cnt == 0){
                unamb_inds[a_ind] = -1;
                continue;
            }   

            // If there is exactly one assignment with a non-zero score then apply that assignment.
            if(cnt == 1){
                unamb_inds[a_ind] = nz_b_ind;
                new_unamb++;
                continue;
            }

            // Or if they all have a score of zero then mark them as unconstrainted or drop if drop_unconstr
            if(cnt == 0){
                if(drop_unconstr){
                    unamb_inds[a_ind] = -1;
                }else{
                    unconstr_mask[a_ind] = 1;
                }
                continue;
            }
        }

        // For variables which are made unconstrained by the
        //  remap so far, greedily assign each i -> j which 
        //  is maximally diagonal, otherwise drop (i.e. i -> -1).
        auto unassigned_j_mask = std::vector<uint8_t>(M, 1);
        for(size_t i = 0; i < N; i++){
            if(unamb_inds[i] >= 0){
                unassigned_j_mask[unamb_inds[i]] = 0;
            }
        }

        for(int64_t i = 0; i < N; i++){
            if(unconstr_mask[i] == 0){
                continue;
            }

            int16_t min_j = -1;
            int16_t min_dist = std::numeric_limits<int16_t>::max();
            for(int64_t j = 0; j < M; j++){
                if(unassigned_j_mask[j] == 1){
                    uint16_t dist = std::abs(i-j);
                    if(dist < min_dist){
                        min_j = j;
                        min_dist = dist;
                    }
                }
            }
            if(min_j != -1){
                unamb_inds[i] = min_j;
                new_unamb++;
            }else{
                unamb_inds[i] = -1;
            }
        }

        // fixed_inds = unamb_inds;
        return new_unamb;
    }

    bool store_remap_push_stack_or_backtrack(){
        bool backtrack = false;
        score_bound = score_remap(fixed_inds, cum_score, var_pairs);

        // cout << "fixed_inds: " << fixed_inds << endl;
        cout << "score_bound: " << score_bound << endl;
        
        // Case: Abandon if the upper bound on the current assignment's 
        //  score is less than the current best score. 
        if(score_bound < best_score * (1.0-tolerance)){
            backtrack = true;
        
        // Case: All vars fixed so store remap. Then backtrack. 
        }else if(all_fixed(fixed_inds)){
            auto remap = std::vector<int16_t>(
                fixed_inds.begin(), fixed_inds.end()); // copy of fixed_inds
            
            // When all inds are fixed, the bound is the true score.
            auto score = score_bound; 
            // score = score_remap(remap, cum_score);
            remaps.push_back(std::make_tuple(score, remap));
            if(score > best_score){
                best_score = score;
                best_remap = &(std::get<1>(remaps[remaps.size() - 1]));
            }
            backtrack = true;
        }

        if(backtrack){
            while(iter_stack.size() > 0){
                SM_StackItem& s = iter_stack[iter_stack.size() - 1];
                
                // Keep popping off stack until the score bound
                //   could feasibly produce a better result.
                if(s.score_bound < best_score * (1.0-tolerance)){
                    iter_stack.pop_back();
                    continue;
                }
                
                // cout << "POP: " << s.i << ", " << s.c << ", " << s.js << ", " << s.fixed_inds << endl;

                // Get fixed inds for the popped iterator
                //  and push its next state to the stack
                auto& old_fixed_inds = s.fixed_inds;
                // Make a copy of old_fixed_inds
                fixed_inds = std::vector<int16_t>(old_fixed_inds.begin(), old_fixed_inds.end());
                fixed_inds[s.i] = s.js[s.c]; // Fix index i to next choice in js.
                s.c += 1;
                if(s.c < s.js.size()){
                    // cout << "PUSHBACK: " << s.i << ", " << s.c << ", " << s.js << ", " << old_fixed_inds << endl << endl;
                    break;
                }else{
                    iter_stack.pop_back();
                }
            }
            if(iter_stack.size() == 0){
                // Case: All iterators exhausted (i.e. Finished)
                // cout << "ALL ITERATORS EXHAUSTED" << endl;
                return true;
            }
            // Case: fixed_inds has been set by popping from stack
        }else{
            // Case: some assignments ambiguous so make next iter.
            // 'fixed_inds' is set to the first choice of i -> j.
            auto [i,js] = get_best_ind_iter(cum_score, fixed_inds);
            if(i != -1){
                // Iterator for rest are pushed to stack. 
                iter_stack.push_back(SM_StackItem(i, 1, js, fixed_inds, score_bound));
                fixed_inds[i] = js[0];
                // cout << "PUSH: " << i << ", " << 1 << ", " << js << ", " << fixed_inds << endl << endl;
            }else{
                // Case: Making iterator failed because only unconstrained
                //  assignments remain. Set all unassigned (-2s) to
                //  unconstrained (-1s)
                for(size_t i = 0; i < fixed_inds.size(); i++){
                    if(fixed_inds[i] == -2) fixed_inds[i] = -1;
                }
            }
        // break;
        }
        return false;
    }
};

// MapProb_setup(){
//     std::vector<int16_t> alignment = std::vector<int16_t>(Da, -2);
//     std::vector<int16_t> fixed_inds = std::vector<int16_t>(N, -2);
//     if(a_fixed_inds != nullptr){
//         for(size_t i = 0; i < a_fixed_inds->size(); i++){
//             fixed_inds[(*a_fixed_inds)[i]] = i;
//         }
//     }

//     float score = 0.0f;
//     float best_score = 0.0f;
//     float score_bound = 0.0f;
//     std::vector<int16_t>* best_remap = nullptr;

//     auto remaps = std::vector<std::tuple<float, std::vector<int16_t>>>();
//     auto iter_stack = std::vector<SM_StackItem>();

// }

// :--------------------------------------------------:
// :  structure_map_generic() :
// :  Generic implementation for structure mapping between two sets of items.
// :  This assumes that a collections of things in A (i.e. literals in logical statements)
// :  have already been paired with match candidates in B, encoded as a SM_MapCandSet object.
// :--------------------------------------------------:


SM_Result structure_map_generic(
    SM_MapCandSet& mapcand_set,
    std::vector<int16_t>* a_fixed_inds,
    bool drop_unconstr, bool drop_no_beta,
    float tolerance=0.0){
        
    size_t N = mapcand_set.N;
    size_t M = mapcand_set.M;
    
    auto& conj_pairs = mapcand_set.conj_pairs;
    size_t Da = mapcand_set.items_a.size();
    size_t Db = mapcand_set.items_b.size();

    MapProblem state(Da, N, M, conj_pairs.size(), a_fixed_inds, 
                     mapcand_set.var_pairs, 
                     tolerance, drop_unconstr, drop_no_beta);

    
    // score_matrices.push_back(ScoreMatrixType(N, M));

    // Outer loop handles generation of iterators over ambiguous
    //  variable assignments. 
    while (true) {
        // Inner loop recalcs score matricies, from current fixed_inds.
        //  Loops multiple times if new scores imply some previously
        //  unfixed variable now has an unambiguous mapping.
        while (true) {
            // Recalculate the score matrix w/ fixed_inds
            _calc_remap_score_matrices(conj_pairs, state.score_matrices, state.fixed_inds);

            // Find the alignment and cumulative score matrix. Required
            //   for when either condition is disjoint (i.e. has an OR()).
            state.alignment = _get_best_alignment(
                Da, Db, conj_pairs, state.score_matrices, state.cum_score,
                mapcand_set.var_pairs
            ); 

            size_t unamb_cnt = state.fill_unambiguous_inds();
            if(unamb_cnt == 0) break;
            // cout << "unamb_cnt: " << unamb_cnt << endl;
        }

        bool do_break = state.store_remap_push_stack_or_backtrack();
        if(do_break) break;                
    }
    


    // cout << "Best Score: " << state.best_score << endl;
    // cout << "Best Remap: " << *state.best_remap << endl;
    auto result = SM_Result(state.best_score, std::move(state.alignment), *state.best_remap, std::move(state.remaps));
    return result;
}




// :--------------------------------------------------:
// :  masks_for_conj_pair() : (private - helps get_masks_for_remap())
// :  Given a group pair and a remap, return masks for the conjuncts in A and B.
// :--------------------------------------------------:

std::tuple<std::vector<uint8_t>, std::vector<uint8_t>> 
    masks_for_conj_pair(SM_ConjPair& gp, std::vector<int16_t>& remap){
    // For a group of literals of the same kind in A and B 
    //  return masks which select literals from A and B 
    //  that would match each other if 'remap' was applied.
    auto matched_As = std::vector<uint8_t>(gp.conj_len_a, 0);
    auto matched_Bs = std::vector<uint8_t>(gp.conj_len_b, 0);
    // float score = 0.0f;

    for(size_t i = 0; i < gp.pairs.size(); i++){
        auto& pair = gp.pairs[i];

        // Skip if this pair has already been matched.
        if(matched_As[pair.index_a] || matched_Bs[pair.index_b]){
            continue;
        }

        // Check if the pair matches.
        VarInds& v_inds_a = pair.var_inds_a;
        VarInds& v_inds_b = pair.var_inds_b;
        bool does_match = true;
        for(size_t j = 0; j < v_inds_a.size(); j++){
            if(remap[v_inds_a[j]] != v_inds_b[j]){
                does_match = false;
                break;
            }
        }

        // Update the masks if pair matches.
        if(does_match){
            matched_As[pair.index_a] = 1;
            matched_Bs[pair.index_b] = 1;
            // score += pair.weight;
        }
    }
    return std::make_tuple(matched_As, matched_Bs);
}

// :--------------------------------------------------:
// :  get_masks_for_remap() :
// :  Given an alignment and remap, recover a mask for each conjunct in A and B.
// :   which omits literals that would not match each other if the remap was applied.
// :--------------------------------------------------:

std::tuple<std::vector<CRE_Type*>, std::vector<CRE_Type*>,
           std::vector<std::vector<uint8_t>>, std::vector<std::vector<uint8_t>>
          > 
    get_masks_for_remap(SM_MapCandSet& map_cand_set, std::vector<int16_t>& alignment, std::vector<int16_t>& remap){
    
    // Empty vectors for masks of conjuncts in A and B.
    auto var_types_a = std::vector<CRE_Type*>(map_cand_set.N, nullptr);
    auto var_types_b = std::vector<CRE_Type*>(map_cand_set.M, nullptr);
    auto item_masks_a = std::vector<std::vector<uint8_t>>(map_cand_set.items_a.size(), std::vector<uint8_t>(0));
    auto item_masks_b = std::vector<std::vector<uint8_t>>(map_cand_set.items_b.size(), std::vector<uint8_t>(0));
    
    for(size_t i = 0; i < map_cand_set.var_pairs.dimension(0); i++){
        for(size_t j = 0; j < map_cand_set.var_pairs.dimension(1); j++){
            if(map_cand_set.var_pairs(i, j).weight >= 0.0f){
                auto&& mutual_type = map_cand_set.var_pairs(i, j).mutual_type;
                var_types_a[i] = mutual_type;
                var_types_b[j] = mutual_type;
            }
        }
    }

    // float score = 0.0f;
    for(size_t k = 0; k < map_cand_set.conj_pairs.size(); k++){
        auto& cp = map_cand_set.conj_pairs[k];
        size_t i = cp.conj_ind_a;
        size_t j = cp.conj_ind_b;

        // Skip if the conjuncts are not aligned.
        if(alignment[i] != j) continue;

        // Get the score and masks for the pair of conjuncts.
        auto [matched_As, matched_Bs] = 
            masks_for_conj_pair(cp, remap);
        item_masks_a[i] = matched_As;
        item_masks_b[j] = matched_Bs;
        // score += p_score;
    }
    return std::make_tuple(var_types_a, var_types_b, item_masks_a, item_masks_b);
}

// ---- SPECIFIC IMPLEMENTATIONS FOR LOGIC OBJECTS ----
// :--------------------------------------------------:
// :  logic_to_map_cands() :
// :  Converts a pair of Logic objects to an SM_MapCandSet object for generic structure mapping.
// :--------------------------------------------------:

void fill_conjunct_like(std::vector<Item>& conj_items, std::vector<size_t>& conj_item_lens, ref<Logic> ls){
    if(ls->kind == CONDS_KIND_AND){
        conj_items.push_back(Item(ls));
        conj_item_lens.push_back(ls->items.size());
        for(auto mbr_item : ls->items){
            if(mbr_item.get_t_id() == T_ID_LOGIC){
                Logic* sub_logic = mbr_item._as<Logic*>();
                assert(sub_logic->kind == CONDS_KIND_OR);
                fill_conjunct_like(conj_items, conj_item_lens, sub_logic);
            } // Skip anything in conjuct that isn't an OR();
        }
    }else{
        for(auto mbr_item : ls->items){
            if(mbr_item.get_t_id() == T_ID_LOGIC){
                Logic* sub_logic = mbr_item._as<Logic*>();
                assert(sub_logic->kind == CONDS_KIND_AND);
                fill_conjunct_like(conj_items, conj_item_lens, sub_logic);
            } else {
                conj_items.push_back(mbr_item);
                conj_item_lens.push_back(1);
            } // Skip anything in disjunct that isn't a literal or a conjuct;
        }
    }
}

void _insert_map(ItemMapType& item_map, int16_t index, Item& item){
    auto it = item_map.find(item);
    if (it != item_map.end()) {
        it->second.push_back(std::tuple<int16_t, Item>(index, item));
    }else{
        std::vector<std::tuple<int16_t, Item>> items = {};
        items.push_back(std::tuple<int16_t, Item>(index, item));
        item_map.insert({item, items});
    }
}

ItemMapType _to_item_map(Item& conj_item){
    ItemMapType item_map = {};

    if(conj_item.get_t_id() == T_ID_LITERAL){
        _insert_map(item_map, -1, conj_item);
    }else{
        auto logic_a = conj_item._as<Logic*>();
        for(size_t index = 0; index < logic_a->items.size(); index++){
            Item& item = logic_a->items[index];
            _insert_map(item_map, index, item);
        }
    }
    return item_map;
}

SM_MappablePair _items_to_mappable_pair(int16_t index_a, int16_t index_b, Item& item_a, Item& item_b){

    if(item_a.get_t_id() == T_ID_LITERAL){
        auto lit_a = item_a._as<Literal*>();
        auto lit_b = item_b._as<Literal*>();
        const VarInds& var_inds_a = lit_a->var_inds;
        const VarInds& var_inds_b = lit_b->var_inds;
        assert(var_inds_a.size() == var_inds_b.size());

        

        // cout << "LITERAL CASE: " << item_a << " -> " << item_b << " ; " << var_inds_a.size() << " , " << var_inds_b.size() <<  endl;
        return SM_MappablePair(lit_a->structure_weight, index_a, index_b,
                               var_inds_a.size(),
                               var_inds_a, var_inds_b);
    }else if(item_a.get_t_id() == T_ID_LOGIC){
        auto logic_a = item_a._as<Logic*>();
        // cout << "LOGIC CASE: " << index_a << " -> " << index_a << endl;
        return SM_MappablePair(logic_a->get_structure_weight(), index_a, index_b);
    }else{
        throw std::runtime_error("NOT IMPLEMENTED");
    }
}


std::vector<SM_MappablePair> make_mappable_pairs(ItemMapType& item_map_a, Item& conj_item_b){
    auto map_pairs = std::vector<SM_MappablePair>();

    cout << "-- MAKE MAPPABLE PAIRS --" << endl;

    // Literals Case: The conjunction-like item is just a literal (probably in an OR)
    if(conj_item_b.get_t_id() == T_ID_LITERAL){
        auto& item_b = conj_item_b;
        auto it = item_map_a.find(item_b);
        if(it != item_map_a.end()){
            for(auto [index_a, item_a] : it->second){
                cout << index_a << " -> " << 0 << " : " << item_a << "," << item_b << endl;
                map_pairs.push_back(_items_to_mappable_pair(index_a, 0, item_a, item_b));
            }
        }

    // Logic Case: The conjunction-like item is a conjunct
    }else{
        auto logic_b = conj_item_b._as<Logic*>();
        for(size_t index_b = 0; index_b < logic_b->items.size(); index_b++){
            auto item_b = logic_b->items[index_b];
            if(item_b.get_t_id() == T_ID_LITERAL){
                auto lit_b = item_b._as<Literal*>();
                auto it = item_map_a.find(item_b);
                if(it != item_map_a.end()){
                    for(auto [index_a, item_a] : it->second){
                        cout << index_a << " -> " << index_b << " : " << item_a << "," << item_b << endl;
                        map_pairs.push_back(_items_to_mappable_pair(index_a, index_b, item_a, item_b));
                    }
                }
            }
        }
    }
    return map_pairs;
}


SM_MapCandSet logic_to_map_cands(ref<Logic> l_a, ref<Logic> l_b){
    auto conj_pairs = std::vector<SM_ConjPair>();
    auto conj_items_a = std::vector<Item>();
    auto conj_item_lens_a = std::vector<size_t>();
    fill_conjunct_like(conj_items_a, conj_item_lens_a, l_a);
    auto conj_items_b = std::vector<Item>();
    auto conj_item_lens_b = std::vector<size_t>();
    fill_conjunct_like(conj_items_b, conj_item_lens_b, l_b);

    auto var_pairs = VarPairMatrix(l_a->vars.size(), l_b->vars.size());
    var_pairs.setConstant(SM_VarPair(nullptr, -1.0f));
    for(size_t i = 0; i < l_a->vars.size(); i++){
        CRE_Type* base_a = l_a->vars[i]->base_type;
        for(size_t j = 0; j < l_b->vars.size(); j++){
            CRE_Type* base_b = l_b->vars[j]->base_type;
            // TODO: If can cast from one to the other
            CRE_Type* mutual_parent = base_a->mutual_parentclass(base_b);
            auto&& entry = var_pairs(i, j);
            if(mutual_parent != nullptr){
                entry.weight = mutual_parent->structure_weight;//std::min(base_a->structure_weight, base_b->structure_weight); 
                entry.mutual_type = mutual_parent;
            }else{
                entry.weight = -1.0f;
                entry.mutual_type = nullptr;
            }
            
        }
    }
    
   // Iterate over all conjuncts in l_a and l_b
   for(size_t i = 0; i < conj_items_a.size(); i++){
        Item& conj_item_a = conj_items_a[i];
        size_t len_a = conj_item_lens_a[i];
        auto item_map = _to_item_map(conj_item_a);
        for(size_t j = 0; j < conj_items_b.size(); j++){
            Item& conj_item_b = conj_items_b[j];
            size_t len_b = conj_item_lens_b[j];
            // cout << "C:" << i << " -> " << j << " : " << conj_item_a << "," << conj_item_b << endl;

            // Use the item map to find all mappable pairs of literals between the conjuncts
            auto pairs = make_mappable_pairs(item_map, conj_item_b);
            // cout << "PAIRS: " << i << "," << j << " SIZE: " << pairs.size() << endl;
            if(pairs.size() > 0){
                conj_pairs.push_back(SM_ConjPair(i, j, len_a, len_b, pairs));
            }
        }
    }
    if(conj_pairs.size() == 0){
        conj_pairs.push_back(SM_ConjPair(0, 0, 0, 0, {}));
    }


    size_t N = l_a->vars.size();
    size_t M = l_b->vars.size();
    return SM_MapCandSet(N, M, conj_items_a, conj_items_b, 
        var_pairs, conj_pairs);
}


// :--------------------------------------------------:
// :  structure_map_logic() :
// :  Specific implementation for structure mapping between Logic objects.
// :--------------------------------------------------:


SM_Result structure_map_logic(
    ref<Logic> l_a, ref<Logic> l_b, 
    std::vector<int16_t>* a_fixed_inds,
    bool drop_unconstr, bool drop_no_beta){
    auto mapcand_set = logic_to_map_cands(l_a, l_b);
    SM_Result result = structure_map_generic(mapcand_set, a_fixed_inds, drop_unconstr, drop_no_beta);
    return result;
}

ref<Logic> antiunify_logic(ref<Logic> l_a, ref<Logic> l_b, 
                           std::vector<int16_t>* a_fixed_inds,
                           float* return_score,
                           uint8_t normalize_kind, 
                           bool drop_unconstr, bool drop_no_beta){

    auto mapcand_set = logic_to_map_cands(l_a, l_b);
    SM_Result result = structure_map_generic(mapcand_set, a_fixed_inds, drop_unconstr, drop_no_beta);
    auto [var_types_a, var_types_b, item_masks_a, item_masks_b] = get_masks_for_remap(mapcand_set, result.alignment, result.best_remap);

    auto new_vars_a = std::vector<ref<Var>>(var_types_a.size(), nullptr);
    for(size_t i = 0; i < var_types_a.size(); i++){
        if(var_types_a[i] != nullptr){
            if(l_a->vars[i]->base_type == var_types_a[i]){
                new_vars_a[i] = l_a->vars[i];
            }else{
                new_vars_a[i] = new_var(l_a->vars[i]->alias, var_types_a[i]);
            }
        }
    }

    float score;
    ref<Logic> output = l_a->masked_copy(new_vars_a, item_masks_a);

    float total_out = output->get_structure_weight();

    if(normalize_kind == AU_NORMALIZE_LEFT){
        float total_a = l_a->get_structure_weight();
        score = total_out / total_a;
    }else if(normalize_kind == AU_NORMALIZE_RIGHT){
        float total_b = l_b->get_structure_weight();
        score = total_out / total_b;
    }else if(normalize_kind == AU_NORMALIZE_MAX){
        float total_a = l_a->get_structure_weight();
        float total_b = l_b->get_structure_weight();
        score = total_out / std::max(std::max(total_a, total_b), 1.0f);
    }
    // cout << "Score: " << score << "," << result.score << endl;
    // result.score = score;
    if(return_score != nullptr){
        *return_score = score;
    }
    // result.keep_masks_a = keep_masks_a;
    // result.keep_masks_b = keep_masks_b;

    // cout << "Keep Masks A: " << endl;
    // for(size_t i = 0; i < keep_masks_a.size(); i++){
    //     cout << "C:" << i << " : " << keep_masks_a[i] << endl;
    // }
    // cout << "Keep Masks B: " << endl;
    // for(size_t i = 0; i < keep_masks_b.size(); i++){
    //     cout << "C:" << i << " : " << keep_masks_b[i] << endl;
    // }

    
    return output;
}


std::ostream& operator<<(std::ostream& os, const SM_Score& obj) {
    os << "{" << obj.ub_score << ", " << obj.beta_score << "}";
    return os;
}


    

} // namespace cre