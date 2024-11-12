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


from cre import CRE_Obj, Fact, NewFact, define_fact, FactSet, Flattener, Vectorizer
import numpy 


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

print(Cat("fluffy", 4));

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
            "type" : "Cat",
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
    return FactSet.from_py(random_cats_dict(N))




# import json
data_path = os.path.join(os.path.split(__file__)[0], "../../data/")
# with open(data_path+'/small_test_state.json') as f:
#     data_dict = json.load(f)
#     print(data_dict)
#     fs = FactSet.from_dict(data_dict)
#     # print(fs)

big_data_path = data_path+"../data/big_untyped_state.json";

# with open(big_data_path) as f:
#     with PrintElapse("read json to str"):
#         data_dict = json.load(f)
#     with PrintElapse("big from dict"):
#         fs = FactSet.from_dict(data_dict)
#     # print(fs)

# with open(big_data_path) as f:
#     with PrintElapse("read big"):
#         json_str = f.read()
#         fs = FactSet.from_json(json_str)

# with PrintElapse("read big file"):
#     fs = FactSet.from_json_file(big_data_path)   

N = 1000

fs = random_cats(N)

with PrintElapse("to_json"):
    json_str = FactSet.to_json(fs)

with PrintElapse("from_json"):
    fs = FactSet.from_json(json_str)

flattener = Flattener()
vectorizer = Vectorizer(7*N, one_hot_nominals=False, encode_missing=True)

with PrintElapse("Flatten"):
    flat_fs = flattener.apply(fs)

# print(flat_fs)

with PrintElapse("Vectorize first run"):
    vectors = vectorizer.apply(flat_fs)

fs2 = random_cats(N)
flat_fs2 = flattener.apply(fs)

with PrintElapse("Vectorize second run"):
    vectors = vectorizer.apply(flat_fs2)

print("START")

for i, val in enumerate(vectors[0]):
    print(vectorizer.invert(i,val))

# print(vectorizer.invert(0,10000))
# print(vectorizer.invert(i+1,val))
# print(vectorizer.invert(-1,val))

for i, val in enumerate(vectors[1]):
    print(vectorizer.invert(i,val))

# print(vectorizer.invert(i+1,val))
# print(vectorizer.invert(-1,val))
# print(vectors)





# print(Cat("fluffy", 4, "blobkin"));
# print(Cat(False, "4"));



