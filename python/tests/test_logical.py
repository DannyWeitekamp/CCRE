import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest

from cre import Equals, Var, AND, OR


def test_logic():
    A = Var(float, "A")
    B = Var(float, "B")
    C = Var(float, "C")

    logic1a = AND(Equals(A, 1), Equals(A, 2))
    logic1o = OR(Equals(A, 1), Equals(A, 2))
    print(logic1a)
    print(logic1o)

    logic2a = AND(Equals(A, 1), Equals(B, 2))
    logic2o = OR(Equals(A, 1), Equals(B, 2))

    print(logic2a)
    print(logic2o)

    print("--------")
    print(AND(Equals(C, 5), logic1a))
    print(AND(Equals(C, 1), logic1o))
    print(OR(Equals(C, 5), logic1a))
    print(OR(Equals(C, 1), logic1o))

    print(AND(Equals(C, 1), Equals(B, 1), Equals(B, 2), logic1o))
    print(OR(Equals(C, 5), Equals(B, 1), logic1a))

    print(AND(logic1o, Equals(C, 1), Equals(B, 1), Equals(B, 2)))
    print(OR(logic1a, Equals(C, 5), Equals(B, 1)))

    print("--------")
    print(AND(Equals(C, 5), logic2a))
    print(AND(Equals(C, 1), logic2o))
    print(OR(Equals(C, 5), logic2a))
    print(OR(Equals(C, 1), logic2o))

    print(AND(Equals(C, 1), Equals(B, 1), Equals(B, 2), logic2o))
    print(OR(Equals(C, 5), Equals(B, 1), logic2a))

    print(AND(logic2o, Equals(C, 1), Equals(B, 1), Equals(B, 2)))
    print(OR(logic2a, Equals(C, 5), Equals(B, 1)))


if __name__ == "__main__":
    import faulthandler; faulthandler.enable()
    
    test_logic()
