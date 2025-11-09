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

    assert(str(A + B) == "A + B")
    assert((A + B) + 1 == "(A + B) + 1")
    assert(1 + (A + B) == "1 + (A + B)")

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
