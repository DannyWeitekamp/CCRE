from test_utils import *
import pytest
import json


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


def _init_py_basic_variants():
    from cre import define_fact, Fact, iFact, NewFact

    Cat = define_fact("Cat", 
        {"name" : (str, {"unique_id" : True}),
         "color" : str,
         "legs" : int,
         "frisky" : bool,
         "x" : float,
         "y" : float
        }
    );

    l_of_d = [{"type" : "Cat", "name" : "snowball", "color" :"white", 
         "legs" :  4, "frisky" : False, "x" : 3.0, "y" : 4.0},
     {"type" : "Cat","name" : "fluffer", "color" :"grey", 
         "legs" :  3, "frisky" : False, "x" : 1.0, "y" : 7.0},
     {"type" : "Cat","name" : "soren", "color" :"white", 
         "legs" :  4, "frisky" : False, "x" : 6.0, "y" : 8.0},
     {"type" : "Cat","name" : "sango", "color" :"brown", 
         "legs" :  4, "frisky" : True, "x" : 3.0, "y" : 2.0},
    ]

    l_of_l = [[v for k,v in d.items() if k != "type"] for d in l_of_d]
    l_of_t = [tuple(v for k,v in d.items() if k != "type") for d in l_of_d]

    d_of_d = {d['name'] : d for d in l_of_d}
    d_of_l = {d['name'] : [v for k,v in d.items() if k != "type"] for d in l_of_d}
    d_of_t = {d['name'] : tuple(v for k,v in d.items() if k != "type") for d in l_of_d}

    ref_fs_d = FactSet()
    for d in l_of_d:
        nt_d = {k:v for k,v in d.items() if k != "type"}
        ref_fs_d.declare(Cat(**nt_d))

    ref_fs_l = FactSet()
    for d in l_of_d:
        nt_d = {k:v for k,v in d.items() if k != "type"}
        ref_fs_l.declare(Fact(*nt_d.values()))

    ref_fs_t = FactSet()
    for d in l_of_d:
        nt_d = {k:v for k,v in d.items() if k != "type"}
        ref_fs_t.declare(iFact(*nt_d.values()))

    d_of_d_et = {key:
        {"EntityType": "Cat", **{k:v for k,v in obj.items() if k != "type"}}
        for key,obj in d_of_d.items()
      }

    return Cat, d_of_d, d_of_l, d_of_t, l_of_d, l_of_l, l_of_t, d_of_d_et, ref_fs_d, ref_fs_l, ref_fs_t


def test_from_py_basic():
    Cat, d_of_d, d_of_l, d_of_t, l_of_d, l_of_l, l_of_t, d_of_d_et, \
        ref_fs_d, ref_fs_l, ref_fs_t = _init_py_basic_variants()
        
    # list of dicts
    l_of_d_fs = FactSet.from_py(l_of_d)
    assert len(l_of_d_fs) == 4
    assert str(l_of_d_fs) == str(ref_fs_d)

    # list of lists
    l_of_l_fs = FactSet.from_py(l_of_l)
    assert len(l_of_l_fs) == 4
    assert str(l_of_l_fs) == str(ref_fs_l)

    # list of tuples
    l_of_t_fs = FactSet.from_py(l_of_t)
    assert len(l_of_t_fs) == 4
    assert str(l_of_t_fs) == str(ref_fs_t)

    # dict of dicts
    d_of_d_fs = FactSet.from_py(d_of_d)
    assert len(d_of_d_fs) == 4
    assert str(d_of_d_fs) == str(ref_fs_d)

    # dict of lists
    d_of_l_fs = FactSet.from_py(d_of_l)
    assert len(d_of_l_fs) == 4
    assert str(d_of_l_fs) == str(ref_fs_l)

    # dict of tuples
    d_of_t_fs = FactSet.from_py(d_of_t)
    assert len(d_of_t_fs) == 4
    assert str(d_of_t_fs) == str(ref_fs_t)

    # dict of dicts
    d_of_d_fs_et = FactSet.from_py(d_of_d_et, type_attr="EntityType")
    assert len(d_of_d_fs_et) == 4
    assert str(d_of_d_fs_et) == str(ref_fs_d)

def test_to_json_basic():
    Cat, d_of_d, d_of_l, d_of_t, l_of_d, l_of_l, l_of_t, d_of_d_et,\
        ref_fs_d, ref_fs_l, ref_fs_t = _init_py_basic_variants()

    py2json = lambda x : json.dumps(x, separators=(',', ':'))

    d_of_di = {str(i):v for i,v in enumerate(d_of_d.values())}
    assert (ref_fs_d.to_json() 
            == py2json(d_of_di))

    assert (ref_fs_d.to_json(unique_id_as_key=True) 
            == py2json(d_of_d))

    d_of_ti = {str(i):v for i,v in enumerate(d_of_t.values())}
    assert (ref_fs_t.to_json() 
            == py2json(d_of_ti))

    assert (ref_fs_d.to_json(doc_as_array=True) 
            == py2json(l_of_d))

    assert (ref_fs_t.to_json(doc_as_array=True) 
            == py2json(l_of_t))

    assert (ref_fs_d.to_json(
                unique_id_as_key=True, 
                type_attr="EntityType"
            )
            == py2json(d_of_d_et))


def test_from_json_basic():
    Cat, d_of_d, d_of_l, d_of_t, l_of_d, l_of_l, l_of_t, d_of_d_et, \
        ref_fs_d, ref_fs_l, ref_fs_t = _init_py_basic_variants()

    py2json = lambda x : json.dumps(x, separators=(',', ':'))

    d_of_di = {str(i):v for i,v in enumerate(d_of_d.values())}
    assert str(FactSet.from_json(py2json(d_of_di))) == \
           str(ref_fs_d)

    assert str(FactSet.from_json(py2json(d_of_d))) == \
           str(ref_fs_d)

    assert str(FactSet.from_json(py2json(l_of_d))) == \
           str(ref_fs_d)

    assert str(FactSet.from_json(py2json(d_of_t))) == \
           str(ref_fs_t)

    assert str(FactSet.from_json(py2json(l_of_t))) == \
           str(ref_fs_t)


    assert (str( FactSet.from_json(
                   py2json(d_of_d_et),
                   type_attr="EntityType"
            )) == str(ref_fs_d))


def _init_py_ref_variants():
    from cre import define_fact, Fact, iFact, NewFact

    Person = define_fact("Person", 
        {"name" : (str, {"unique_id" : True}),
         "mom" : "Person",
         "dad" : "Person",
         "best_bud" : "Person",
        }
    );

    l_of_d = [
        {"type" : "Person", "name" : "Ricky",
          "mom" : "@Mumsy", "dad" : "@Daddy'o", "best_bud" : "4"}, #i.e. Earl
        {"type" : "Person", "name" : "Daddy'o",
         "mom" : None, "dad" : "Gramps", "best_bud" : "@2"}, #i.e. Mumsy
        {"type" : "Person", "name" : "Mumsy",
         "mom" : None, "dad" : None, "best_bud" : 1}, #i.e. Daddy'o
        {"type" : "Person", "name" : "Gramps",
          "mom" : None, "dad" : None, "best_bud" : "@Daddy'o"},
        {"type" : "Person", "name" : "Earl",
          "mom" : None, "dad" : None, "best_bud" : None},
    ]

    l_of_l = [[v for k,v in d.items() if k != "type"] for d in l_of_d]
    l_of_t = [tuple(v for k,v in d.items() if k != "type") for d in l_of_d]

    d_of_d = {d['name'] : d for d in l_of_d}
    d_of_l = {d['name'] : [v for k,v in d.items() if k != "type"] for d in l_of_d}
    d_of_t = {d['name'] : tuple(v for k,v in d.items() if k != "type") for d in l_of_d}

    return Person, l_of_d, l_of_l, l_of_t, d_of_d, d_of_l, d_of_t

def test_from_py_ref():
    Person, l_of_d, l_of_l, l_of_t, d_of_d, d_of_l, d_of_t = \
        _init_py_ref_variants()

    print(FactSet.from_py(l_of_d), "\n")

    # These one won't work, since named refs like "@Mumsy" won't work
    with pytest.raises(ValueError):
        print(FactSet.from_py(l_of_l), "\n")
    with pytest.raises(ValueError):
        print(FactSet.from_py(l_of_t), "\n")


    print(FactSet.from_py(d_of_d), "\n")
    print(FactSet.from_py(d_of_l), "\n")

    # TODO: This one is weird
    print(FactSet.from_py(d_of_t), "\n")





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
    test_from_py_basic()
    test_to_json_basic()
    test_from_json_basic()
    test_from_py_ref()
    
