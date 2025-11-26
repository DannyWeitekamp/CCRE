

@njit(cache=True)
def conds_structure_map(c_a, c_b, a_fixed_inds,
     drop_unconstr, drop_no_beta):
    Da, Db = len(c_a.dnf), len(c_b.dnf)
    N, M = len(c_a.vars), len(c_b.vars)

    
    lit_groups = make_lit_groups(c_a, c_b)
    
    fixed_inds = np.full(N,-2,dtype=np.int16)
    fixed_inds[:len(a_fixed_inds)] = a_fixed_inds
    remaps = List.empty_list(align_remap_type)
    iter_stack = List.empty_list(stack_item_type)
    it = None
    c = 0
    score, best_score, score_bound = u2(0), u2(0), u2(0)
    best_result = None


    # Outer loop handles generation of iterators over ambiguous
    #  variable assignments.
    while(True):
        # Inner loop recalcs score matricies, from current fixed_inds.
        #  Loops multiple times if new scores imply some previously
        #  unfixed variable now has an unambiguous mapping.
        while(True):
            # Recalculate the score matrix w/ fixed_inds
            remap_score_matrices, beta_score_matrices = (
                _calc_remap_score_matrices(
                    lit_groups, (Da, Db, N, M), fixed_inds
            ))

            # Find the alignment and cumulative score matrix. Required
            #  for when either condition is disjoint (i.e. has an OR()).
            alignment, cum_score_matrix, cum_beta_matrix = (
                _get_best_alignment(remap_score_matrices, beta_score_matrices)
            )
            # print(cum_score_matrix)
            # print(alignment, fixed_inds)

            # Look for unambiguous remaps in the new matrix
            fixed_inds, unamb_cnt = get_unambiguous_inds(
                cum_score_matrix, cum_beta_matrix, fixed_inds, 
                drop_unconstr, drop_no_beta)
            
            if(unamb_cnt == 0):
                break

        # print("new_unambiguious", fixed_inds, unamb_cnt)
        
        score_bound = score_remap(
            np.argmax(cum_score_matrix, axis=1),
            cum_score_matrix, cum_beta_matrix
        )

        # print(f"BEST={best_score}", f"BOUND={score_bound}")
        backtrack = False

        # Case: Abandon if the upper bound on the current assignment's 
        #   score is less than the current best score. 
        if(score_bound < best_score):
            backtrack = True
        
        
        # Case: All vars fixed so store remap. Then backtrack. 
        elif(np.all(fixed_inds != -2)):
            # Future NOTE: If remap is recalculated w/ 
            #  align_greedy then unmatched symbols are dropped, 
            #  but they are kept if just use fixed_inds.
            # remap = align_greedy(cum_score_matrix)
            remap = fixed_inds.copy()
            remaps.append((score_bound, alignment, remap))
            score, keep_mask_a, keep_mask_b = _score_and_mask(
                c_a, c_b, lit_groups, alignment, remap)

            if(score > best_score):
                best_result = (alignment, remap, keep_mask_a, keep_mask_b)
                best_score = score

            backtrack = True

        if(backtrack):
            if(len(iter_stack) == 0):
                # Case: All iterators exhausted (i.e. Finished)
                break

            while(len(iter_stack) > 0):
                i, c, js, old_fixed_inds = iter_stack.pop()
                # print("POP", i, c, js, old_fixed_inds)
                fixed_inds = old_fixed_inds.copy()
                fixed_inds[i] = js[c]
                c += 1
                if(c < len(js)):
                    iter_stack.append((i, c, js, old_fixed_inds))
                    # print("PUSH", i, c, js, old_fixed_inds)
                    break

            # Case: fixed_inds has been set by popping from stack
        else:
            # Case: some assignments ambiguous so make next iter.
            #  'fixed_inds' is set to the first choice of i -> j.
            #  Iterator for rest are pushed to stack. 
            (i,js) = get_best_ind_iter(cum_score_matrix, fixed_inds)
            iter_stack.append((i, 1, js, fixed_inds.copy()))
            fixed_inds[i] = js[0]
            # print("PUSH", i, 1, js, fixed_inds.copy())

    if(best_result is not None):
        alignment, remap, keep_mask_a, keep_mask_b = best_result
        return best_score, alignment, remap, keep_mask_a, keep_mask_b
    else:
        # Make return consistent with no valid mapping
        alignment = np.arange(len(c_a.dnf),dtype=np.int16)
        remap = np.full(N, -1, dtype=np.int16)
        max_conj_len_a = max([len(conj) for conj in c_a.dnf])
        max_conj_len_b = max([len(conj) for conj in c_b.dnf])
        keep_mask_a = np.zeros((len(c_a.dnf), max_conj_len_a), dtype=np.uint8)
        keep_mask_b = np.zeros((len(c_b.dnf), max_conj_len_b), dtype=np.uint8)
        return best_score, alignment, remap, keep_mask_a, keep_mask_b


@njit
def _calc_remap_score_matrices(lit_groups, shape, a_fixed_inds):
    # Fill in a score and beta matrices with dimensions:
    #    Da : number of conjucts in A's DNF         
    #    Db : number of conjucts in B's DNF
    #    N : number of variables in A
    #    M : number of variables in B
    # Each cell (da, db, i, j) indicates the total number of 
    #  variable references (within literals) that are valid
    #  if variable mapping i -> j is performed. `a_fixed_inds`
    #  forces a subset of the possible mappings to be considered
    #  the only possible variable assignments. 
    #  `remap_score_matrices` counts all valid references
    #  and `beta_score_matrices` counts only those associated
    #  with literals of two variables---counted seperately
    #  so that a tight upper bound on the true remap score (the 
    #  number of vars + literals retained) can be recovered.

    Da, Db, N, M = shape
    remap_score_matrices = np.zeros((Da, Db, N, M), dtype=np.uint16)
    beta_score_matrices = np.zeros((Da, Db, N, M), dtype=np.uint16)

    b_fixed_inds = np.full(M, -2, dtype=np.int16)
    for i,j in enumerate(a_fixed_inds):
        if(j != -2):
            b_fixed_inds[j] = i

    # For each conjunct pair
    for i, groups_i in enumerate(lit_groups):
        for j, groups_ij in enumerate(groups_i):
            score_matrix = remap_score_matrices[i, j]
            beta_matrix = beta_score_matrices[i, j]

            for _, var_inds_a, _, var_inds_b in groups_ij: 
                # For each literal in A
                for v_inds_a in var_inds_a:
                    ind_a0 = v_inds_a[0]
                    fix_b0 = a_fixed_inds[ind_a0]

                    # Alpha Case
                    if(len(v_inds_a) == 1):
                        for v_inds_b in var_inds_b:
                            ind_b0 = v_inds_b[0]
                            fix_a0 = b_fixed_inds[ind_b0]
                            if((fix_b0 == -2 or fix_b0 == ind_b0) and
                               (fix_a0 == -2 or fix_a0 == ind_a0)):
                                score_matrix[ind_a0, ind_b0] += 1

                    # Beta Case
                    else:
                        ind_a1 = v_inds_a[1]
                        fix_b1 = a_fixed_inds[ind_a1]
                        for v_inds_b in var_inds_b:
                            ind_b0 = v_inds_b[0]
                            ind_b1 = v_inds_b[1]
                            fix_a0 = b_fixed_inds[ind_b0]
                            fix_a1 = b_fixed_inds[ind_b1]
                            if((fix_b0 == -2 or fix_b0 == ind_b0) and
                               (fix_a0 == -2 or fix_a0 == ind_a0) and
                               (fix_b1 == -2 or fix_b1 == ind_b1) and
                               (fix_a1 == -2 or fix_a1 == ind_a1) ):
                                score_matrix[ind_a0, ind_b0] += 1
                                score_matrix[ind_a1, ind_b1] += 1
                                beta_matrix[ind_a0, ind_b0] += 1
                                beta_matrix[ind_a1, ind_b1] += 1

    return remap_score_matrices, beta_score_matrices

@njit(cache=True)
def get_unambiguous_inds(cum_score_matrix, cum_beta_matrix,
        a_fixed_inds, drop_unconstr, drop_no_beta):
    unamb_inds = a_fixed_inds.copy()
    unconstr_mask = np.zeros(len(a_fixed_inds),dtype=np.uint8)
    new_unamb = 0
    N, M = cum_score_matrix.shape
    for a_ind in range(N):
        # Don't touch if already assigned  
        if(a_fixed_inds[a_ind] != -2):
            continue

        # Find any assignments with non-zero score
        cnt = 0
        beta_cnt = 0
        nz_b_ind = -1
        for b_ind in range(M):
            if(cum_score_matrix[a_ind,b_ind] != 0):
                cnt += 1
                nz_b_ind = b_ind
            if(cum_beta_matrix[a_ind,b_ind] != 0):
                beta_cnt += 1

        # If drop_no_beta=True and no beta literals 
        #  support any assignment to a_ind and then drop.
        if(drop_no_beta and beta_cnt == 0):
            unamb_inds[a_ind] = -1

        # If there is exactly one assignment with a non-zero
        #  score then apply that assignment.
        elif(cnt == 1):
            scores_a_to_b = cum_score_matrix[:,nz_b_ind]
            # if(np.max(scores_a_to_b) == cum_score_matrix[a_ind,nz_b_ind]):
            if(np.sum(scores_a_to_b != 0) == 1):
                new_unamb += 1
                unamb_inds[a_ind] = nz_b_ind
        # Or if they all have a score of zero then mark
        #  them as unconstrainted or drop if drop_unconstr
        elif(cnt == 0):
            if(drop_unconstr):
                unamb_inds[a_ind] = -1
            else:
                unconstr_mask[a_ind] = 1


            
    # For variables which are made unconstrained by the
    #  remap so far, greedily assign each i -> j which 
    #  is maximally diagonal, otherwise drop (i.e. i -> -1).
    unassigned_j_mask = np.ones(M,dtype=np.uint8)
    unassigned_j_mask[unamb_inds[unamb_inds >= 0]] = 0
    # print(cum_score_matrix)
    # print(": ", unamb_inds)
    # print("unconstrained i:", np.nonzero(unconstr_mask)[0])
    # print("unassigned j:", np.nonzero(unassigned_j_mask)[0])
    for i in np.nonzero(unconstr_mask)[0]:
        min_j  = -1
        min_dist = 100000
        for j, unassigned in enumerate(unassigned_j_mask):
            if(unassigned):
                dist = abs(i-j)
                if(dist < min_dist):
                    min_j = j
                    min_dist = dist

        if(min_j != -1):
            unassigned_j_mask[min_j] = 0
            unamb_inds[i] = min_j
        else:
            unamb_inds[i] = -1
    # print(":>", unamb_inds)
    # print()

    return unamb_inds, new_unamb

@njit(cache=True)
def _get_best_alignment(remap_score_matrices, beta_score_matrices):
    # Use remap_score_matrices to find the best alignment of
    #  conjuncts between A and B. Find the cumulative
    #  score and beta matricies from this alignment.

    Da, Db, N, M = remap_score_matrices.shape

    if(N == 1 and M == 1):
        return (np.zeros(1, dtype=np.int16),
                remap_score_matrices[0,0],
                beta_score_matrices[0,0])

    # Fill in a matrix which captures the best remap 
    #  for each conjunct pair. This is an upper bound
    #  on the score contribution of the best remap of the
    #  the best alignment.
    upb_alignment_matrix = np.zeros((Da, Db), dtype=np.uint16)
    remaps = np.empty((Da, Db, N), dtype=np.int16)
    for i in range(Da):
        for j in range(Db):
            remap = align_greedy(remap_score_matrices[i,j])
            upb_alignment_matrix[i, j] = score_remap(remap,
                remap_score_matrices[i,j], beta_score_matrices[i,j])

    # Assume that the best alignment is just the one that
    #  maximizes these upper bounds
    alignment = align_greedy(upb_alignment_matrix)

    cum_score_matrix = np.zeros((N, M), dtype=np.uint16)
    cum_beta_matrix = np.zeros((N, M), dtype=np.uint16)
    for i,j in enumerate(alignment):
        cum_score_matrix += remap_score_matrices[i,j]
        cum_beta_matrix += beta_score_matrices[i,j]

    
    return alignment, cum_score_matrix, cum_beta_matrix

@njit(cache=True)
def get_matched_masks(group, remap):
    # For a group of literals of the same kind in A and B 
    #  return masks which select literals from A and B 
    #  that would match each other if 'remap' was applied.

    _, var_inds_a, _, var_inds_b = group

    a_inds_remapped = np.empty(len(var_inds_a[0]),dtype=np.int16)
    matched_As = np.zeros(len(var_inds_a),dtype=np.uint8)
    matched_Bs = np.zeros(len(var_inds_b),dtype=np.uint8)
    
    for i, v_inds_a in enumerate(var_inds_a):
        # Apply this mapping for A -> B 
        for ix, var_ind in enumerate(v_inds_a):
           a_inds_remapped[ix] = remap[var_ind]

        # Greedily assign literals in remapped A to literals in B
        for j, v_inds_b in enumerate(var_inds_b):
            if(not matched_Bs[j] and np.array_equal(a_inds_remapped, v_inds_b)):
                matched_As[i] = 1
                matched_Bs[j] = 1
                break
    return matched_As, matched_Bs

@njit(cache=True)
def _score_and_mask_conj(conj_a, conj_b, groups, remap):
    # Assuming conjucts _a and _b are paired, return the 
    #  mapping score and masks (indexed by d_ind, c_ind)
    #  of literals kept when `conj_a` and `conj_b` are
    #  intersected with `remap` applied to their variables.

    keep_mask_a = np.zeros(len(conj_a), dtype=np.uint8)
    keep_mask_b = np.zeros(len(conj_b), dtype=np.uint8)

    score = u2(0)
    for group in groups: 
        c_inds_a, _, c_inds_b, _ = group
        mm_a, mm_b = get_matched_masks(group, remap)

        for keep_it, c_ind in zip(mm_a, c_inds_a):
            keep_mask_a[c_ind] = keep_it
            score += keep_it

        for keep_it, c_ind in zip(mm_b, c_inds_b):
            keep_mask_b[c_ind] = keep_it

    return score, keep_mask_a, keep_mask_b

@njit
def _score_and_mask(c_a, c_b, lit_groups, alignment, remap):
    # Apply _score_and_mask_conj() over each conjunction in
    #  the dnfs of A and B, pairing them according to alignment.
    #  Return the accumulated score and masks.

    max_conj_len_a = max([len(conj) for conj in c_a.dnf])
    max_conj_len_b = max([len(conj) for conj in c_b.dnf])
    keep_mask_a = np.zeros((len(c_a.dnf), max_conj_len_a), dtype=np.uint8)
    keep_mask_b = np.zeros((len(c_b.dnf), max_conj_len_b), dtype=np.uint8)
    
    # +1 for every Var which is kept 
    score = np.sum(remap != -1, dtype=np.uint16)

    # +1 for every literal across all disjunctions kept 
    for i, j in enumerate(alignment):
        groups_ij = lit_groups[i][j]
        
        conj_a, conj_b = c_a.dnf[i], c_b.dnf[j],
        _score, _keep_mask_a, _keep_mask_b = _score_and_mask_conj(
            conj_a, conj_b, groups_ij, remap
        )

        keep_mask_a[i,:len(_keep_mask_a)] = _keep_mask_a
        keep_mask_b[j,:len(_keep_mask_b)] = _keep_mask_b
        
        score += _score
    return score, keep_mask_a, keep_mask_b

# NOTES:
'''

lit_group_ij : 
  Essentially pairs a conjunct from A with a conjunct from B.
  Contains c_inds_a, var_inds_a, c_inds_b, var_inds_b
    where:
        c_inds : For each literal the index in which it appears in its conjunct
        var_inds: for each literal the indicies of the 1 or 2 vars that contribute to the literal
    
    var_inds is used in the hottest loops... it might make sense to keep them contiguous
    There are obvious constant width optimizations by treating alpha and beta separately,
     especially since each group_ij is either strictly alpha or beta, so we can lift the
     decision of which algorithm we are using into base of the group_ij loop.

Adding score on == -2 (i.e. not yet fixed):
  It is unclear I add score here, it would appear the score decreases then on each fixing
  there is probably an optimization where the score grows on each fixing, avoiding the need
  to reallocate a new map matrix on each fixture 





'''
