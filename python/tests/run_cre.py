import os, glob, importlib.util

# Monkey-Patch import cre for testing purposes
build_path = os.path.join(os.path.split(__file__)[0], "../../build/")
ext_path = glob.glob(os.path.normpath(build_path) + "/cre.*")[0]
spec = importlib.util.spec_from_file_location("cre", ext_path)
module = importlib.util.module_from_spec(spec)

import time
class PrintElapse():
    def __init__(self, name):
        self.name = name
    def __enter__(self):
        self.t0 = time.time_ns()/float(1e6)
    def __exit__(self,*args):
        self.t1 = time.time_ns()/float(1e6)
        print(f'{self.name}: {self.t1-self.t0:.6f} ms')


from cre import CRE_Obj, Fact, NewFact, define_fact

fact = NewFact(None, 2, 3, "moose")
print("AFTER")
print(fact, "len=", len(fact))


with PrintElapse("define cat"):
    Cat = define_fact("Cat", {
        "name" : "str",
        "legs" : "int",
    })

Dog = define_fact("Dog", {
    "name" : str,
    "legs" : int,
})

# print(Cat("fluffy", 4));

class PyCat():
    def __init__(self, *args, **kwargs):
        self.name = kwargs['name']
        self.legs = kwargs['legs']

with PrintElapse("CATx1000"):
    for i in range(1000):
        Cat(name="fluffy", legs=4);

with PrintElapse("PyCatx1000"):
    for i in range(1000):
        PyCat(name="fluffy", legs=4);

with PrintElapse("dictx1000"):
    for i in range(1000):
        dict(name="fluffy", legs=4);


# print(Cat("fluffy", 4, "blobkin"));
# print(Cat(False, "4"));



