from test_utils import *
import pytest



def test_declare_retract():
    pass

def test_modify():
    pass

def test_retact_error():
    pass

def test_iter_get_facts():
    pass

def test_indexer():
    pass

def test_mem_leaks_basic():
    pass

def test_mem_leaks_refs():
    pass

def test_long_hash():
    pass

def test_from_py():
    Cat = define_fact("Cat", 
        {"name" : str,
         "color" : str,
         "legs" : int,
         "frisky" : bool,
         "x" : float,
         "y" : float
        }
    );

    lst = [{"type" : "Cat", "name" : "snowball", "color" :"white", 
         "legs" :  4, "frisky" : False, "x" : 3.0, "y" : 4.0},
     {"type" : "Cat","name" : "fluffer", "color" :"grey", 
         "legs" :  3, "frisky" : False, "x" : 1.0, "y" : 7.0},
     {"type" : "Cat","name" : "soren", "color" :"white", 
         "legs" :  4, "frisky" : False, "x" : 6.0, "y" : 8.0},
     {"type" : "Cat","name" : "sango", "color" :"brown", 
         "legs" :  4, "frisky" : True, "x" : 3.0, "y" : 2.0},
    ]

    no_type_lst = [{k:v for k,v in d.items() if k != "type"} for d in lst]
    ref_fs = FactSet()
    for nt_d in no_type_lst:
        ref_fs.declare(Cat(**nt_d))

    lst_fs = FactSet.from_py(lst)
    assert len(lst_fs) == 4
    assert str(lst_fs) == str(ref_fs)

    dct = {x['name'] : x for x in lst}
    dct_fs = FactSet.from_py(dct)
    assert len(dct_fs) == 4
    assert str(lst_fs) == str(ref_fs)

    



def test_to_json():
    pass

def test_from_json():
    pass




@pytest.mark.benchmark(group="memset")
def test_b_declare_1000(benchmark):
    pass

@pytest.mark.benchmark(group="memset")
def test_b_retract_1000(benchmark):
    pass

@pytest.mark.benchmark(group="memset")
def test_get_facts_1000(benchmark):
    pass

@pytest.mark.benchmark(group="memset")
def test_get_facts_by_id_1000(benchmark):
    pass


if __name__ == "__main__":
    # fs = random_cats(10)
    # print(fs)

    # test_declare_retract()
    # test_modify()
    # test_retact_error()
    # test_iter_get_facts()
    # test_indexer()
    # test_mem_leaks_basic()
    # test_mem_leaks_refs()
    # test_long_hash()
    test_from_py()
    # test_from_json()
    # test_to_json()
