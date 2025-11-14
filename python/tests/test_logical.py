import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest

from cre import Var, AND, OR, Not, Exists, Bound, Opt, Tuple, define_fact


Person = define_fact("Person", 
    {"id": {"type" : str, "unique_id" : True},
     "money": float,
     "mom": "Person",
     "dad": "Person",
     "best_bud": "Person"
    }
);

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
    for typ in [int, float]:
        A = Var(typ, "A")
        B = Var(typ, "B")
        C = Var(typ, "C")

        _0,_1,_2,_3 = typ(0),typ(1),typ(2),typ(3)

        # Equals
        assert(str(A == B) == "A == B")
        assert(str(A == _1) == f"A == {_1}")
        assert((A == B)(_2,_3) == False)
        assert((A == B)(_2,_2) == True)
        assert((A == _1)(_1) == True)
        assert((A == _1)(_2) == False)

        # Not Equals
        assert(str(A != B) == "A != B")
        assert(str(A != _1) == f"A != {_1}")
        assert((A != B)(_2,_3) == True)
        assert((A != B)(_2,_2) == False)
        assert((A != _1)(_1) == False)
        assert((A != _1)(_2) == True)
        
        # Less than
        assert(str(A < B) == "A < B")
        assert(str(A < _1) == f"A < {_1}")
        assert((A < B)(_2,_3) == True)
        assert((A < B)(_3,_2) == False)
        assert((A < _1)(_0) == True)
        assert((A < _1)(_2) == False)
        
        # Greater than
        assert(str(A > B) == "A > B")
        assert(str(A > _1) == f"A > {_1}")
        assert((A > B)(_3,_2) == True)
        assert((A > B)(_2,_3) == False)
        assert((A > _1)(_2) == True)
        assert((A > _1)(_0) == False)
        
        # Less than or equal
        assert(str(A <= B) == "A <= B")
        assert(str(A <= _1) == f"A <= {_1}")
        assert((A <= B)(_2,_3) == True)
        assert((A <= B)(_2,_2) == True)
        assert((A <= B)(_3,_2) == False)
        assert((A <= _1)(_1) == True)
        assert((A <= _1)(_2) == False)
        
        # Greater than or equal
        assert(str(A >= B) == "A >= B")
        assert(str(A >= _1) == f"A >= {_1}")
        assert((A >= B)(_3,_2) == True)
        assert((A >= B)(_2,_2) == True)
        assert((A >= B)(_2,_3) == False)
        assert((A >= _1)(_2) == True)
        assert((A >= _1)(_0) == False)

        # Addition
        assert(str(A + B) == "A + B")
        assert(str(A + _1) == f"A + {_1}")
        assert(str(_1 + A) == f"{_1} + A")
        assert((A + B)(_2,_3) == 5)
        assert(((A + B) + _1)(_2,_3) == 6)
        assert((_1 + (A + B))(_2,_3) == 6)
        
        # Subtraction
        assert(str(A - B) == "A - B")
        assert(str(A - _1) == f"A - {_1}")
        assert(str(_1 - A) == f"{_1} - A")
        assert((A - B)(_2,_3) == -1)
        assert(((A - B) - _1)(_2,_3) == -2)
        assert((_1 - (A - B))(_2,_3) == 2)
        
        # Multiplication
        assert(str(A * B) == "A * B")
        assert(str(A * _2) == f"A * {_2}")
        assert(str(_2 * A) == f"{_2} * A")
        assert((A * B)(_2,_3) == 6)
        assert(((A * B) * _2)(_2,_3) == 12)
        assert((_2 * (A * B))(_2,_3) == 12)
        
        # # True division
        assert(str(A / B) == "A / B")
        assert(str(A / _2) == f"A / {_2}")
        assert(str(_2 / A) == f"{_2} / A")
        print((A / B)(_2,_3))
        assert((A / B)(_2,_3) == pytest.approx(2/3))
        assert(((A / B) / _2)(_2,_3) == pytest.approx(1/3))
        assert((_2 / (A / B))(_2,_3) == 3)
        
        # Floor division
        assert(str(A // B) == "A // B")
        assert(str(A // _2) == f"A // {_2}")
        assert(str(_2 // A) == f"{_2} // A")
        assert((A // B)(_2,_3) == 0)
        assert(((A // B) // _2)(_2,_3) == 0)
        assert((_2 // (A // B + _1))(_2,_3) == 2)
        
        # Modulo
        assert(str(A % B) == "A % B")
        assert(str(A % _2) == f"A % {_2}")
        assert(str(_2 % A) == f"{_2} % A")
        assert((A % B)(_2,_3) == _2)
        assert(((A % B) % _2)(_2,_3) == 0)
        assert((_2 % (A % B))(_2,_3) == 0)
        
        # Power
        assert(str(A ** B) == "A ** B")
        assert(str(A ** _2) == f"A ** {_2}")
        assert(str(_2 ** A) == f"{_2} ** A")
        assert((A ** B)(_2,_3) == 8)
        assert(((A ** B) ** _2)(_2,_3) == 64)
        assert((_2 ** (A ** B))(_2,_3) == 256)
        
        # Negation
        assert(str(-A) == "-A")
        assert(str(-(A + B)) == "-(A + B)")
        assert((-A)(_2) == -2)
        assert((-(A + B))(_2,_3) == -5)

        # Inversion - Note: Use ~A because "not" cannot be overloaded
        assert(str(~(A + B)) == "~(A + B)")
        print((~(A + B))(_2,_3))
        assert((~(A + B))(_2,_3) == False)

    S0, S1 = Var(str, "S0"), Var(str, "S1")
    print(str(S0 + S1))
    assert(str(S0 + S1) == "S0 + S1")
    assert((S0 + S1)("A", "B") == "AB")


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

def test_var_types():
    A = Var(float, "A")
    B = Not(float, "B")
    C = Exists(float, "C")
    D = Bound(float, "D")
    E = Opt(float, "E")
    print(repr(A))
    print(repr(B))
    print(repr(C))
    print(repr(D))
    print(repr(E))

def test_semantic_var():
    A = Var(float,"A")
    _A = Var(float,"A")
    iA = Var(int,"A")
    uA = Var("A")
    print(repr(uA))

    print("-----------")
    f = A + _A
    
    assert(f(1) == 2)
    with pytest.raises(ValueError):
        f = A + iA

    f = A + uA
    print(repr(f))
    print(f(1))
    
    print(repr(uA))
    f = uA + uA
    print(f)
    print(f(2))

    f = uA + A
    print(AND(f))
    print(f(2))

    # print("f REFCNT", f.get_refcount())
    # print(f(1))
    # print("f REFCNT", f.get_refcount())
    # print("REFCNT", A.get_refcount())
    # f = None
    # print("REFCNT", A.get_refcount())
    # A = None
    # _A = None

def test_anon_vars_to_names():
    # print(Var(Person).money)
    # print(Var(Person))
    # print(Var(Person).money==100.0)
    # print(Var(Person).money<100.0)
    # print("--------------------------------")
    _vars = [Var(Person).money==100.0]
    for i in range(20):
        _vars.append(Var(Person))
    _vars.append(Var(Person).money==100.0)
    print(_vars)
    print("LL", len(_vars))

    print(AND(*_vars))

def test_fact_literals():
    A = Var(str,"A")
    z0 = Var(str, 0)

    t = Tuple(1,"A","B");
    print(AND(t));

    t = Tuple(1,A,"B");
    print(AND(t));

    t = Tuple(1,z0,"B");
    print(AND(t));

    print(AND(Person(id="bob", money=100.0)));

    P = Var(Person)
    print(AND(Person(id=A, money=100.0)));

    print(AND(P0:=Var(Person(id="bob", money=100.0))))

    print(AND(P0:=Var(Person), P0.money < 105.0, P0 == Person(id="bob", money=100.0)))

    print(OR(1, 2, 3))

    print("--------")
    print(AND(P0:=Var(Person), P0.money==OR(100.5, 200, 300)))
    print(P0.money==OR(100.5, 200, 300))


    # print(f(1))




if __name__ == "__main__":
    import faulthandler; faulthandler.enable()
    

    # test_anon_vars_to_names()
    test_basic_logic()
    # test_logic()
    # test_arith()
    # test_name_resolution()
    # test_var_types()
    # test_semantic_var()
    test_fact_literals()
