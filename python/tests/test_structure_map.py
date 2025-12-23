import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest
import math

from cre import Var, AND, OR, Not, Exists, Bound, Opt, Tuple, FactType


def test_anti_unify():
    x, y, z = Var(float,'x'), Var(float,'y'), Var(float,'z')
    a, b, c, d = Var(float,'a'), Var(float,'b'), Var(float,'c'), Var(float,'d')

    # -------------
    # : Single Conjunction Case
    
    c1 = AND(x < y, y < z, y < z, z != x, y != 0) 
    c2 = AND(a < b, b < c, b < c, b < c, c != a, b != 0, d != 0)

    c12_ref = AND(x < y, y < z, y < z, z != x, y != 0)

    c12 = c1.antiunify(c2) 
    print("c12:")
    print(c12)
    # return
    with PrintElapse("antiunify"):
        for i in range(100):
            c12 = c1.antiunify(c2) 
    
    print(str(c12_ref))
    print(str(c12))
    assert str(c12) == str(c12_ref)
    # raise ValueError()

    # -------------
    # : Disjunction of Conjunctions Case
    #   (test alignment of conjuncts)
    print("---")
    c1 = OR(AND(x < y, z != x, y != 0), 
            AND(x < y, z == x, y != 7), 
            AND(x > y, z != x, y != 2)  
         )
    
    c2 = OR(AND(a < b, c == a, b != 7, d > 0), # 1
            AND(a < b, c != a, b != 0),           # 0
            AND(a > b, c != a, b != 0, d != 7)  # 2
         )
        
    c12_ref = OR(AND(x < y, z != x, y != 0), 
                 AND(x < y, z == x, y != 7), 
                 AND(x > y, z != x))

    c12, score = c1.antiunify(c2, return_score=True) #conds_antiunify(c1,c2)

    print(str(c12_ref))
    print(str(c12))
    print(score)
    assert str(c12) == str(c12_ref)
    assert math.isclose(score, 11./12., abs_tol=1e-6)
    print("---")
    print(AND(x, y, z))
    print("---")
    # -------------
    # : Test fix_same_var and fix_same_alias

    # Baseline case
    c1 = AND(x < y, y < z, y < z, z != x, y != 0) 
    c2 = AND(x < y, z < y, z < y, x != z, z != 0) 
    c12_ref = AND(x, y, z, y != 0, y < z, y < z)
    c12, score = c1.antiunify(c2, return_score=True, fix_same_var=False)
    print(c1)
    print(c2)
    print(str(c12_ref))
    print(str(c12))
    print("SCORE", score)
    # print_str_diff(str(c12),str(c12_ref))
    assert str(c12) == str(c12_ref)
    assert score == 6./8.

    # With fix_same_var=True
    c1 = AND(x < y, y < z, y < z, z != x, y != 0)
    c2 = AND(x < y, z < y, z < y, x != z, z != 0)
    c12_ref = AND(x, y, z, x < y)
    c12, score = c1.antiunify(c2, return_score=True, fix_same_var=True)
    print(str(c12_ref))
    print(str(c12))
    print("SCORE", score)
    assert str(c12) == str(c12_ref)
    assert score == 4./8.

    # With fix_same_alias=True
    X, Y, Z = Var(float,'x'), Var(float,'y'), Var(float,'z')
    c1 = AND(x < y, y < z, y < z, z != x, y != 0) 
    c2 = AND(X < Y, Z < Y, Z < Y, X != Z, Z != 0) 
    c12_ref = AND(x, y, z, x < y)
    c12, score = c1.antiunify(c2, return_score=True, fix_same_alias=True)
    print(str(c12_ref))
    print(str(c12))
    print("SCORE", score)
    assert str(c12) == str(c12_ref)
    assert score == 4./8.

    # -------------
    # : Edge cases

    # Edge case: variables with no supporting literals
    c1 = AND(x, y, z)
    c2 = AND(X, Y, Z)
    c12_ref = AND(x, y, z)
    c12, score = c1.antiunify(c2, return_score=True, fix_same_alias=True)
    print("SCORE", score)
    print("A", c12_ref)
    print("B", c12)
    
    assert str(c12) == str(c12_ref)
    assert score == 1.

if __name__ == "__main__":
    test_anti_unify()