import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest

from cre import Var, AND, OR

import time
class PrintElapse():
    def __init__(self, name):
        self.name = name
    def __enter__(self):
        self.t0 = time.time_ns()/float(1e6)
    def __exit__(self,*args):
        self.t1 = time.time_ns()/float(1e6)
        print(f'{self.name}: {self.t1-self.t0:.6f} ms')

def test_basic_logic():
    A = Var(float, "A")
    B = Var(float, "B")
    C = Var(float, "C")

    logic1a = AND(A == 1, A == 2)
    logic1o = OR(A == 1, A == 2)
    print(logic1a)
    print(logic1o)

    logic2a = AND(A == 1, B == 2)
    logic2o = OR(A == 1, B == 2)

    print(logic2a)
    print(logic2o)

    print("--------")
    print(AND(C == 5, logic1a))
    print(AND(C == 1, logic1o))
    print(OR(C == 5, logic1a))
    print(OR(C == 1, logic1o))

    print(AND(C == 1, B == 1, B == 2, logic1o))
    print(OR(C == 5, B == 1, logic1a))

    print(AND(logic1o, C == 1, B == 1, B == 2))
    print(OR(logic1a, C == 5, B == 1))

    print("--------")
    print(AND(C == 5, logic2a))
    print(AND(C == 1, logic2o))
    print(OR(C == 5, logic2a))
    print(OR(C == 1, logic2o))

    print(AND(C == 1, B == 1, B == 2, logic2o))
    print(OR(C == 5, B == 1, logic2a))

    print(AND(logic2o, C == 1, B == 1, B == 2))
    print(OR(logic2a, C == 5, B == 1))

def test_arith():
    A = Var(float, "A")
    B = Var(float, "B")
    C = Var(float, "C")

    # Equals
    assert(str(A == B) == "A == B")
    assert(str(A == 1) == "A == 1")
    assert((A == B)(2,3) == False)
    assert((A == B)(2,2) == True)
    assert((A == 1)(1) == True)
    assert((A == 1)(2) == False)

    # Not Equals
    assert(str(A != B) == "A != B")
    assert(str(A != 1) == "A != 1")
    assert((A != B)(2,3) == True)
    assert((A != B)(2,2) == False)
    assert((A != 1)(1) == False)
    assert((A != 1)(2) == True)
    
    # Less than
    assert(str(A < B) == "A < B")
    assert(str(A < 1) == "A < 1")
    assert((A < B)(2,3) == True)
    assert((A < B)(3,2) == False)
    assert((A < 1)(0) == True)
    assert((A < 1)(2) == False)
    
    # Greater than
    assert(str(A > B) == "A > B")
    assert(str(A > 1) == "A > 1")
    assert((A > B)(3,2) == True)
    assert((A > B)(2,3) == False)
    assert((A > 1)(2) == True)
    assert((A > 1)(0) == False)
    
    # Less than or equal
    assert(str(A <= B) == "A <= B")
    assert(str(A <= 1) == "A <= 1")
    assert((A <= B)(2,3) == True)
    assert((A <= B)(2,2) == True)
    assert((A <= B)(3,2) == False)
    assert((A <= 1)(1) == True)
    assert((A <= 1)(2) == False)
    
    # Greater than or equal
    assert(str(A >= B) == "A >= B")
    assert(str(A >= 1) == "A >= 1")
    assert((A >= B)(3,2) == True)
    assert((A >= B)(2,2) == True)
    assert((A >= B)(2,3) == False)
    assert((A >= 1)(2) == True)
    assert((A >= 1)(0) == False)

    # Addition
    assert(str(A + B) == "A + B")
    assert(str(A + 1) == "A + 1")
    assert(str(1 + A) == "1 + A")
    assert((A + B)(2,3) == 5)
    assert(((A + B) + 1)(2,3) == 6)
    assert((1 + (A + B))(2,3) == 6)
    
    # Subtraction
    assert(str(A - B) == "A - B")
    assert(str(A - 1) == "A - 1")
    assert(str(1 - A) == "1 - A")
    assert((A - B)(2,3) == -1)
    assert(((A - B) - 1)(2,3) == -2)
    assert((1 - (A - B))(2,3) == 2)
    
    # Multiplication
    assert(str(A * B) == "A * B")
    assert(str(A * 2) == "A * 2")
    assert(str(2 * A) == "2 * A")
    assert((A * B)(2,3) == 6)
    assert(((A * B) * 2)(2,3) == 12)
    assert((2 * (A * B))(2,3) == 12)
    
    # # True division
    assert(str(A / B) == "A / B")
    assert(str(A / 2) == "A / 2")
    assert(str(2 / A) == "2 / A")
    assert((A / B)(2,3) == 2/3)
    assert(((A / B) / 2)(2,3) == 1/3)
    assert((2 / (A / B))(2,3) == 3)
    
    # Floor division
    assert(str(A // B) == "A // B")
    assert(str(A // 2) == "A // 2")
    assert(str(2 // A) == "2 // A")
    assert((A // B)(2,3) == 0)
    assert(((A // B) // 2)(2,3) == 0)
    assert((2 // (A // B + 1))(2,3) == 2)
    
    # Modulo
    assert(str(A % B) == "A % B")
    assert(str(A % 2) == "A % 2")
    assert(str(2 % A) == "2 % A")
    assert((A % B)(2,3) == 2)
    assert(((A % B) % 2)(2,3) == 0)
    assert((2 % (A % B))(2,3) == 0)
    
    # Power
    assert(str(A ** B) == "A ** B")
    assert(str(A ** 2) == "A ** 2")
    assert(str(2 ** A) == "2 ** A")
    assert((A ** B)(2,3) == 8)
    assert(((A ** B) ** 2)(2,3) == 64)
    assert((2 ** (A ** B))(2,3) == 256)
    
    # Negation
    assert(str(-A) == "-A")
    assert(str(-(A + B)) == "-(A + B)")
    
    

    print(A < B)
    print(~(A < B))


def test_name_resolution():
    # Test that names will be resolved just from local variable names
    #   e.g. C:=Var(float) should produce a Var(float, "C") 
    conds = AND(
      C:=Var(float), C == 5, 
      A:=Var(float), A == 1, 
      B:=Var(float), B == 2
    )

    with PrintElapse("init 1000 lits"):
        for i in range(1000):
            conds = AND(C:=Var(float), C == 5)

    with PrintElapse("init 1000 lits named"):
        for i in range(1000):
            conds = AND(C:=Var(float, "C"), C == 5)

    print(conds)


if __name__ == "__main__":
    import faulthandler; faulthandler.enable()
    
    # test_logic()
    test_arith()
    # test_name_resolution()
