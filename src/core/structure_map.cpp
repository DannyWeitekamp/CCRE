
#include "../include/item.h"
#include "../include/logic.h"
#include "../include/structure_map.h"
#include <vector>
#include <unsupported/Eigen/CXX11/Tensor>
#include <Eigen/Dense>




namespace cre {


void fill_conjunct_like(std::vector<Item>& conj_items, ref<Logic> ls){
    if(ls->kind == CONDS_KIND_AND){
        conj_items.push_back(Item(ls));
        for(auto mbr_item : ls->items){
            if(mbr_item.get_t_id() == T_ID_LITERAL){
                Logic* sub_logic = mbr_item._as<Logic*>();
                assert(sub_logic->kind == CONDS_KIND_OR);
                fill_conjunct_like(conj_items, sub_logic);
            } // Skip anything in conjuct that isn't an OR();
        }
    }else{
        for(auto mbr_item : ls->items){
            if(mbr_item.get_t_id() == T_ID_LOGIC){
                Logic* sub_logic = mbr_item._as<Logic*>();
                assert(sub_logic->kind == CONDS_KIND_AND);
                fill_conjunct_like(conj_items, sub_logic);
            } else if(mbr_item.get_t_id() == T_ID_LITERAL){
                conj_items.push_back(mbr_item);
            } // Skip anything in disjunct that isn't a literal or a conjuct;
        }
    }
}

// Custom comparator functor
// struct ItemComparator {
//     bool operator()(const Item& a, const Item& b) const {
//         uint16_t t_id_a = a.get_t_id();
//         if (t_id_a != b.get_t_id()) {
//             return t_id_a < b.get_t_id();
//         }
//         if(t_id_a == T_ID_LITERAL){
//             auto lit_a = a._as<Literal*>();
//             auto lit_b = b._as<Literal*>();
//             if (lit_a->kind != lit_b->kind) {
//                 return lit_a->kind < lit_b->kind;
//             }
//             if (lit_a->negated != lit_b->negated) {
//                 return lit_a->negated < lit_b->negated;
//             }
//             if(lit_a->kind == LIT_KIND_FUNC) {
//                 auto func_a = lit_a._as<Func*>();
//                 auto func_b = lit_b._as<Func*>();
//                 if (func_a->origin_data != func_b->origin_data) {
//                     return func_a->origin_data < func_b->origin_data;
//                 }
//             }
//             if(lit_a->kind == LIT_KIND_FACT) {
//             if (lit_a->vars.size() != lit_b->vars.size()) {
//                 return lit_a->vars.size() < lit_b->vars.size();
//             }
//             for(size_t i = 0; i < lit_a->vars.size(); i++){
//             }
//         }

//         return a.name < b.name;
//     }
// };

// void _check_map(HashMap<Item, std::vector<Item>> item_map, Item& item){
//     auto it = item_map.find(item);
//     return 
//     if (it != item_map.end()) {
//         it->second.push_back(item);
//     }
//     std::vector<Item> items = {}
//     items.push_back(item);
//     item_map.insert({item_map, items});
// }
typedef HashMap<Item, std::vector<std::tuple<int16_t, Item>>> ItemMapType;

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
            auto item = logic_a->items[index];
            _insert_map(item_map, index, item);
        }
    }
    
    return item_map;
}
SM_MappablePair _items_to_mappable_pair(int16_t index_a, int16_t index_b, Item& item_a, Item& item_b){

    if(item_a.get_t_id() == T_ID_LITERAL){
        auto lit_a = item_a._as<Literal*>();
        auto lit_b = item_b._as<Literal*>();
        std::vector<uint16_t>& var_inds_a = lit_a->var_inds;
        std::vector<uint16_t>& var_inds_b = lit_b->var_inds;
        assert(var_inds_a.size() == var_inds_b.size());
        return SM_MappablePair(1.0, index_a, index_b,
                               var_inds_a.size(),
                               var_inds_a, var_inds_b);
    }else if(item_a.get_t_id() == T_ID_LOGIC){
        return SM_MappablePair(1.0, index_a, index_b);
    }else{
        throw std::runtime_error("NOT IMPLEMENTED");
    }

    
    
}


std::vector<SM_MappablePair> make_mappable_pairs(ItemMapType& item_map_a, Item& conj_item_b){
    auto map_pairs = std::vector<SM_MappablePair>();

    // Literals Case: The conjunction-like item is just a literal (probably in an OR)
    if(conj_item_b.get_t_id() == T_ID_LITERAL){
        auto& item_b = conj_item_b;
        auto it = item_map_a.find(item_b);
        if(it != item_map_a.end()){
            for(auto [index_a, item_a] : it->second){
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
                        map_pairs.push_back(_items_to_mappable_pair(index_a, index_b, item_a, item_b));
                    }
                }
            }
        }
    }
    return map_pairs;
}


std::vector<SM_GroupPair> make_group_pairs(ref<Logic> l_a, ref<Logic> l_b){
   auto group_pairs = std::vector<SM_GroupPair>();
   auto conj_items_a = std::vector<Item>();
   fill_conjunct_like(conj_items_a, l_a);
   auto conj_items_b = std::vector<Item>();
   fill_conjunct_like(conj_items_b, l_b);


   // Iterate over all conjuncts in l_a and l_b
   for(size_t i = 0; i < conj_items_a.size(); i++){
        Item& conj_item_a = conj_items_a[i];
        auto item_map = _to_item_map(conj_item_a);
        for(size_t j = 0; j < conj_items_b.size(); j++){
            Item& conj_item_b = conj_items_a[i];

            // Use the item map to find all mappable pairs of literals between the conjuncts
            auto pairs = make_mappable_pairs(item_map, conj_item_b);
            if(pairs.size() > 0){
                group_pairs.push_back(SM_GroupPair(i, j, pairs));
            }
        }
    }
    return group_pairs;
}

/*
SM_Result structure_map_logic(
    ref<Logic> l_a, ref<Logic> l_b, 
    std::vector<int16_t>& a_fixed_inds,
    bool drop_unconstr, bool drop_no_beta){

    size_t N = l_a->vars.size();
    size_t M = l_b->vars.size();
    auto group_pairs = make_group_pairs(l_a, l_b);
    
    std::vector<int16_t> fixed_inds = std::vector<int16_t>(N, -2);
    for(size_t i = 0; i < a_fixed_inds.size(); i++){
        fixed_inds[a_fixed_inds[i]] = i;
    }

    auto it = nullptr;
    size_t c = 0;
    float score = 0.0f;
    float best_score = 0.0f;
    float score_bound = 0.0f;
    float best_result = nullptr;

    auto remaps = std::vector<std::vector<int16_t>>();
    auto iter_stack = std::vector<SM_StackItem>();

    ScoreMatrixType cum_score(N, M);
    std::vector<ScoreMatrixType> score_matrices;
    score_matrices.push_back(ScoreMatrixType(N, M));

    // Outer loop handles generation of iterators over ambiguous
    //  variable assignments. 
    while (true) {
        // Inner loop recalcs score matricies, from current fixed_inds.
        //  Loops multiple times if new scores imply some previously
        //  unfixed variable now has an unambiguous mapping.
        while (true) {
            // Recalculate the score matrix w/ fixed_inds
            _calc_remap_score_matrices(score_matrices, lit_groups, fixed_inds);

            // Find the alignment and cumulative score matrix. Required
            //   for when either condition is disjoint (i.e. has an OR()).
            _get_best_alignment(cum_score, score_matrices);

            auto [fixed_inds, unamb_cnt] = get_unambiguous_inds(cum_score, fixed_inds, drop_unconstr, drop_no_beta);

            if(unamb_cnt == 0){
                break;
            }
        }

        bool backtrack = false;
        score_bound = score_remap(fixed_inds, cum_score);

        // Case: Abandon if the upper bound on the current assignment's 
        //  score is less than the current best score. 
        if(score_bound < best_score){
            backtrack = true;
        
        // Case: All vars fixed so store remap. Then backtrack. 
        else if(all_fixed(fixed_inds)){
            auto remap = fixed_inds.copy();
            remaps.push_back(remap);
            score = score_remap(remap, cum_score);
            if(score > best_score){
                best_score = score;
                best_result = remap;
            }
            backtrack = true;
        }

        (i, js) = get_best_ind_iter(cum_score, fixed_inds);
        iter_stack.push_back((i, 1, js, fixed_inds.copy()));
    }
}

bool all_fixed(std::vector<int16_t>& fixed_inds){
    return std::all_of(fixed_inds.begin(), fixed_inds.end(), [](int16_t x){ return x != -2; });
}

void _calc_remap_score_matrices(
    std::vector<ScoreMatrixType>& score_matrices, 
    std::vector<SM_GroupPair>& group_pairs, 
    std::vector<uint16_t>& a_fixed_inds){


    for(size_t i = 0; i < group_pairs.size(); i++){
        auto gp = group_pairs[i];
        auto score_matrix = score_matrices[i];
        
        // Reset Score Matrix
        score_matrix.setConstant(SM_Score(0.0,0.0));

    // for(size_t i = 0; i < groups_ij.size(); i++){
        for(SM_MappablePair& mp : groups_ij){
            switch(mp.n_vars){
                case 1:
                {
                    uint16_t ind_a0 = mp.var_inds_a.inds[0];
                    uint16_t ind_b0 = mp.var_inds_b.inds[0];
                    uint16_t fix_b0 = a_fixed_inds[ind_a0];
                    uint16_t fix_a0 = b_fixed_inds[ind_b0];
                    if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                       (fix_a0 == -2 or fix_a0 == ind_a0)){
                        score_matrix[ind_a0, ind_b0].ub_score += 1;
                    }

                    // for(ItemVarInds& v_inds_a : map_pair.var_inds_a){
                    //     uint16_t ind_a0 = v_inds_a.ind0;
                    //     uint16_t fix_b0 = a_fixed_inds[ind_a0];
                    //     for(auto v_inds_b : var_inds_b){
                    //         uint16_t ind_b0 = v_inds_b.ind0;
                    //         uint16_t fix_a0 = b_fixed_inds[ind_b0];
                    //         if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                    //            (fix_a0 == -2 or fix_a0 == ind_a0)){
                    //             score_matrix[ind_a0, ind_b0].ub_score += 1;
                    //         }
                    //     }
                    // }
                    break;
                }
                case 2:
                {
                    uint16_t ind_a0 = mp.var_inds_a.inds[0];
                    uint16_t ind_b0 = mp.var_inds_b.inds[0];
                    uint16_t ind_a1 = mp.var_inds_a.inds[1];
                    uint16_t ind_b1 = mp.var_inds_b.inds[1];
                    uint16_t fix_b0 = a_fixed_inds[ind_a0];
                    uint16_t fix_a0 = b_fixed_inds[ind_b0];
                    uint16_t fix_b1 = a_fixed_inds[ind_a1];
                    uint16_t fix_a1 = b_fixed_inds[ind_b1];
                    if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                       (fix_a0 == -2 or fix_a0 == ind_a0) &&
                       (fix_b1 == -2 or fix_b1 == ind_b1) &&
                       (fix_a1 == -2 or fix_a1 == ind_a1) ){
                        score_matrix[ind_a0, ind_b0].ub_score += 1;
                        score_matrix[ind_a1, ind_b1].ub_score += 1;
                        score_matrix[ind_a0, ind_b0].beta_score += 1;
                        score_matrix[ind_a1, ind_b1].beta_score += 1;
                    }
                    
                    

                    // for(ItemVarInds& v_inds_a : map_pair.var_inds_a){
                    //     uint16_t ind_a0 = v_inds_a.ind0;
                    //     uint16_t ind_a1 = v_inds_a.ind1;
                    //     uint16_t fix_b0 = a_fixed_inds[ind_a0];
                    //     uint16_t fix_b1 = a_fixed_inds[ind_a1];
                    //     for(auto v_inds_b : var_inds_b){
                    //         uint16_t ind_b0 = v_inds_b.ind0;
                    //         uint16_t ind_b1 = v_inds_b.ind1;
                    //         uint16_t fix_a0 = b_fixed_inds[ind_b0];
                    //         uint16_t fix_a1 = b_fixed_inds[ind_b1];
                    //         if((fix_b0 == -2 or fix_b0 == ind_b0) &&
                    //            (fix_a0 == -2 or fix_a0 == ind_a0) &&
                    //            (fix_b1 == -2 or fix_b1 == ind_b1) &&
                    //            (fix_a1 == -2 or fix_a1 == ind_a1) ){
                    //             score_matrix[ind_a0, ind_b0].ub_score += 1;
                    //             score_matrix[ind_a1, ind_b1].ub_score += 1;
                    //             score_matrix[ind_a0, ind_b0].beta_score += 1;
                    //             score_matrix[ind_a1, ind_b1].beta_score += 1;
                    //         }
                    //     }
                    // }
                }
                default:
                    throw std::runtime_error("NOT IMPLEMENTED");

            }
        }
    }
    return;
}

std::vector<int16_t> _get_best_alignment(
    std::vector<ScoreMatrixType>& score_matrices, 
    ScoreMatrixType& cum_score){

    
    size_t N = cum_score.dimension(0);
    size_t M = cum_score.dimension(1);

    
    if(score_matrices.size() == 1){
        cum_score = score_matrices[0];
        return std::vector<int16_t>(1, 0);
    }else{
        throw std::runtime_error("NOT IMPLEMENTED");
    }

    // cum_score.setConstant(SM_Score(0.0,0.0));

    // auto upb_alignment_matrix = Eigen::MatrixXd<uint16_t, Eigen::RowMajor>(Da, Db);
    // auto remaps = Eigen::MatrixXd<int16_t, Eigen::RowMajor>(score_matrices.size(), N);

    // for(size_t i = 0; i < Da; i++){
    //     for(size_t j = 0; j < Db; j++){
    //         score_matrices[i][j].ub_score += score_matrices[i][j].ub_score;
    //     }
    // }


}

std::tuple<std::vector<int16_t>, uint16_t> 
  get_unambiguous_inds(
    ScoreMatrixType& cum_score,
    std::vector<int16_t>& a_fixed_inds,
    bool drop_unconstr, bool drop_no_beta){

    size_t N = cum_score.dimension(0);
    size_t M = cum_score.dimension(1);
    auto unamb_inds = a_fixed_inds.copy();
    auto unconstr_mask = std::vector<uint8_t>(a_fixed_inds.size());
    uint16_t new_unamb = 0;
    

    for(size_t a_ind = 0; a_ind < N; a_ind++){
        // Don't touch if already assigned  
        if(a_fixed_inds[a_ind] != -2){
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
        if(drop_no_beta and beta_cnt == 0){
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

    for(size_t i = 0; i < N; i++){
        if(unconstr_mask[i] == 0){
            continue;
        }

        uint16_t min_j = -1;
        uint16_t min_dist = std::numeric_limits<uint16_t>::max();
        for(size_t j = 0; j < M; j++){
            if(unassigned_j_mask[j] == 1){
                uint16_t dist = abs(i-j);
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
    return std::make_tuple(unamb_inds, new_unamb);
}

float score_remap(
    const std::vector<int16_t>& remap,
    ScoreMatrixType& score_matrix){

    float score = 0.0f;
    for(size_t i = 0; i < remap.size(); i++){
        score += score_matrix(i, remap[i]).ub_score - (score_matrix(i, remap[i]).beta_score >> 1) + (score_matrix(i, remap[i]).ub_score != 0);
    }
    return score;
}


std::tuple<std::vector<bool>, std::vector<bool>> get_matched_masks(
    SM_MappablePair& group, const std::vector<int16_t>& remap){
    
    uint16_t n_vars = group.n_vars;
    auto& var_inds_a = group.var_inds_a;
    auto& var_inds_b = group.var_inds_b;
    auto a_inds_remapped = std::vector<int16_t>(var_inds_a[0].size());
    auto matched_As = std::vector<bool>(var_inds_a.size(), false);
    auto matched_Bs = std::vector<bool>(var_inds_b.size(), false);
    

    for(size_t i = 0; i < var_inds_a.size(); i++){
        auto& v_inds_a = var_inds_a[i];

        if(n_vars <= 2){
            a_inds_remapped[0] = remap[v_inds_a.ind0];
            if(n_vars == 2){
                a_inds_remapped[1] = remap[v_inds_a.ind1];
            }
        }else{
            for(size_t ix = 0; ix < n_vars; ix++){
                a_inds_remapped[ix] = remap[v_inds_a.inds[ix]];
            }
        }
        
        for(size_t j = 0; j < var_inds_b.size(); j++){
            auto& v_inds_b = var_inds_b[j];
            if(matched_Bs[j]){
                continue;
            }
            if(std::equal(a_inds_remapped.begin(), a_inds_remapped.end(), v_inds_b.begin())){
                matched_As[i] = true;
                matched_Bs[j] = true;
                break;
            }
        }
    }
    return std::make_tuple(matched_As, matched_Bs);
}
std::tuple<float, std::vector<bool>, std::vector<bool>>  
   _score_and_mask_conj(size_t conj_len_a, size_t conj_len_b, 
                        std::vector<SM_MappablePair>& groups, 
                        const std::vector<int16_t>& remap){

    auto keep_mask_a = std::vector<bool>(conj_len_a, false);
    auto keep_mask_b = std::vector<bool>(conj_b.size(), false);
    float score = 0.0f;
    for(size_t i = 0; i < groups.size(); i++){
        SM_MappablePair& group = groups[i];
        auto [mm_a, mm_b] = get_matched_masks(group, remap);

        for(size_t j = 0; j < mm_a.size(); j++){
            bool keep = mm_a[j];
            keep_mask_a[group.lit_inds_a[j]] = keep;
            score += keep;
        }

        for(size_t j = 0; j < mm_b.size(); j++){
            bool keep = mm_b[j];
            keep_mask_b[group.lit_inds_b[j]] = keep;
        }
    }
    return std::make_tuple(score, keep_mask_a, keep_mask_b);
}


std::tuple<float, std::vector<uint8_t>, std::vector<uint8_t>> 
    _score_and_mask(ref<Logic> l_a, ref<Logic> l_b, 
                    std::vector<std::vector<SM_MappablePair>>& lit_groups, 
                    std::vector<int16_t>& alignment, 
                    std::vector<int16_t>& remap){

    
    auto keep_mask_a = std::vector<uint8_t>(c_a.dnf.size(), 0);
    auto keep_mask_b = std::vector<uint8_t>(c_b.dnf.size(), 0);
    

    // +1 for every variable which is kept
    float score = 0.0f;
    for(size_t i = 0; i < remap.size(); i++){
        score += remap[i] != -1 ? 1 : 0;
    }

    // // +1 for every literal across all disjunctions kept 
    // for(uint16_t i = 0; i < alignment.size(); i++){
    //     uint16_t j = alignment[i];
    //     auto [_score, keep_mask_a, keep_mask_b] = _score_and_mask_conj(
    //         c_a.dnf[i], c_b.dnf[j], lit_groups[i][j], remap);
    //     score += _score;
    // }
    return std::make_tuple(score, keep_mask_a, keep_mask_b);
}
    */

} // namespace cre