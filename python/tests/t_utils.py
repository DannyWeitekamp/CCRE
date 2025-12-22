import os, sys, glob, importlib.util

# Monkey-Patch import cre for testing purposes
build_path = os.path.join(os.path.split(__file__)[0], "../../release/")
ext_path = glob.glob(os.path.normpath(build_path) + "/cre.*")[0]
spec = importlib.util.spec_from_file_location("cre", ext_path)

# print("build_path: ", build_path)
# print("ext_path: ", ext_path)
# print("spec: ", spec)
cre_module = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = cre_module
spec.loader.exec_module(cre_module)

# print(cre_module)

import time
class PrintElapse():
    def __init__(self, name):
        self.name = name
    def __enter__(self):
        self.t0 = time.time_ns()/float(1e6)
    def __exit__(self,*args):
        self.t1 = time.time_ns()/float(1e6)
        print(f'{self.name}: {self.t1-self.t0:.6f} ms')


from cre import CRE_Obj, Fact, define_fact, FactSet

# print(CRE_Obj, Fact, define_fact, FactSet)

CatType = define_fact("Cat", 
    {"id" : int,
     "name" : str,
     "color" : str,
     "legs" : int,
     "frisky" : bool,
     "x" : float,
     "y" : float
    }
);

def random_cats_dict(N):
    from random import choice, uniform
    names = ["fluffer", "soren", "sango", "snowball", "crabcake"]
    colors = ["black", "white", "calico", "orange", "brown"]
    friskies = [True, False]
    d = {}
    for i in range(N):
        d[str(i)] = {
            "type" : CatType,
            "id" : i,
            "name" : choice(names),
            "color" : choice(colors),
            "legs" : 4,
            "frisky" : choice(friskies),
            "x" : uniform(0.0, 1.0),
            "y" : uniform(0.0, 1.0),
        }
    return d

def random_cats(N):
    return FactSet.from_dict(random_cats_dict(N))
