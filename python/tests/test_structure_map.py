import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest

from cre import Var, AND, OR, Not, Exists, Bound, Opt, Tuple, FactType, items_equal


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
    assert isclose(score, 11./12.)
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


def test_antiunify_fact_types():
    Animal = FactType("Animal", 
        {"id" : int,
         "name" : str,
         "color" : str,
         "x" : float,
         "y" : float
        }
    );
    Cat = FactType("Cat", 
        {"ears" : str,
         "fur" : str,
         "frisky" : bool,
        },
        inherits_from=Animal
    )
    Fish = FactType("Fish", 
        {"fins" : str,
         "scales" : str,
        },
        inherits_from=Animal
    )

    
    c1,c2,c3 = Var(Cat), Var(Cat), Var(Cat)
    f1,f2,f3 = Var(Fish), Var(Fish), Var(Fish)
    a1,a2,a3 = Var(Animal, "c1"), Var(Animal, "c2"), Var(Animal, "c3")
    cat_trio = AND(c1.color == c2.color, c1.x < c2.x, c1.frisky == True, c3.fur == c2.fur) 
    fish_trio = AND(f1.color == f2.color, f1.x < f2.x, f1.scales == "blue", f3.fins == f2.fins) 

    print("items_equal", items_equal(c1.color == c2.color, f1.color == f2.color, False, True))
    # return

    animal_trio_ref = AND(a1.color == a2.color, a1.x < a2.x, a3)
    animal_trio, score = cat_trio.antiunify(fish_trio, return_score=True) 
    print(cat_trio)
    print(fish_trio)
    print(animal_trio)
    print(animal_trio_ref)
    print("SCORE", score)
    assert str(animal_trio) == str(animal_trio_ref)
    # assert isclose(score, 1.0)

if __name__ == "__main__":
    # test_anti_unify()
    test_antiunify_fact_types()