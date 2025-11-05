import faulthandler; faulthandler.enable()
import os, sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from t_utils import *
import pytest


from cre import And, Add, AddInts, Var, peak_locals, resolve_alias, resolve_alias_fast, do_nothing


import time
class PrintElapse():
    def __init__(self, name):
        self.name = name
    def __enter__(self):
        self.t0 = time.time_ns()/float(1e6)
    def __exit__(self,*args):
        self.t1 = time.time_ns()/float(1e6)
        print(f'{self.name}: {self.t1-self.t0:.6f} ms')

Q1 = Var(int)
Q2 = Var(int)
Q3 = Var(int)

def test_basic_func():
    print(Add(1,2))
    A = Var(int, "A")
    B = Var(int, "B")

    print(A + B)

    print(A,B)
    print(Add(A,B))

    print(Add(A,B)+1)


    print(A)


    peak_locals()

    Add(C := Var(int), D := Var(int))

    # Cold start
    with PrintElapse("do_nothing"):
        do_nothing()
    resolve_alias(C)
    resolve_alias_fast(C)
    resolve_alias_fast(Q1)

    C = Var(int)
    with PrintElapse("local"):
        resolve_alias(C)

    with PrintElapse("global"):
        resolve_alias(Q2)

    print(C, Q2)

    C = Var(int)
    with PrintElapse("local-fast"):
        resolve_alias_fast(C)

    with PrintElapse("global-fast"):
        resolve_alias_fast(Q3)

    print(C, Q3)

    with PrintElapse("do_nothing"):
        do_nothing()



if __name__ == "__main__":
    import faulthandler; faulthandler.enable()
    
    test_basic_func()


