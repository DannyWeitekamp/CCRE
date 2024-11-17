from test_utils import *
import pytest



def _fixed_cat_fs():
    from cre import define_fact

    Cat = define_fact("_Cat", 
        {"id" : (int, {"unique_id" : True,}),
         "name" : (str, {"visible" : True,}),
         "color" : (str, {"visible" : True,}),
         "legs" : (int, {"visible" : True,}),
         "frisky" : (bool, {"visible" : False,}),
         "x" : (float, {"visible" : True,}),
         "y" : (float, {"visible" : True,})
        }
    );

    fs = FactSet()
    fs.declare(Cat(0, "fluffer", "grey", 4, False, 3.0, 4.0))
    fs.declare(Cat(1, "soren", "white", 4, True, 12.0, 9.0))
    fs.declare(Cat(2, "sango", "brown", 4, True, 13.0, 8.0))
    fs.declare(Cat(3, "crabcake", "orange", 4, False, 3.0, 7.0))
    return Cat, fs
    

def _fixed_person_fs():
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
    fs = FactSet.from_py(l_of_d)

    return Person, fs


def _flatten_state_variants(fs):
    from cre import Flattener
    flattener = Flattener()
    flat_fs0 = flattener.apply(fs)

    flattener = Flattener(add_exist_stubs=True)
    flat_fs1 = flattener.apply(fs)

    flattener = Flattener(use_vars=True)
    flat_fs2 = flattener.apply(fs)

    flattener = Flattener(use_vars=True, add_exist_stubs=True)
    flat_fs3 = flattener.apply(fs)
    return flat_fs0, flat_fs1, flat_fs2, flat_fs3

def _do_flattener_tests(fs, n_vis):
    flat_fs0, flat_fs1, flat_fs2, flat_fs3 = _flatten_state_variants(fs)
    assert len(flat_fs0) == n_vis*len(fs)
    assert len(flat_fs1) == (n_vis+1)*len(fs)

    # Refcount sanity check (flat_fs has one, fact has one)
    for fact in flat_fs1:
        assert fact.get_refcount() == 2;

    
    assert len(flat_fs2) == (n_vis)*len(fs)
    assert len(flat_fs3) == (n_vis+1)*len(fs)

    # Refcount sanity check (flat_fs has one, fact has one)
    for fact in flat_fs3:
        assert fact.get_refcount() == 2;
    

def test_Flattener_basic():
    Cat, fs = _fixed_cat_fs()
    _do_flattener_tests(fs, 6)

def test_Flattener_refs():
    Person, fs = _fixed_person_fs()
    _do_flattener_tests(fs, 4)


def test_Vectorizer_basic():
    from cre import Vectorizer
    Cat, fs = _fixed_cat_fs()
    flat_fs0, flat_fs1, flat_fs2, flat_fs3 = _flatten_state_variants(fs)

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs0)

    print(nom, cont)
    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs1)

    print(nom, cont)
    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))


def test_Vectorizer_refs():
    from cre import Vectorizer
    Person, fs = _fixed_person_fs()
    flat_fs0, flat_fs1, flat_fs2, flat_fs3 = _flatten_state_variants(fs)
    # flat_fs0, flat_fs1 = test_Flattener_refs()    

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs0)

    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))

    print(nom, cont)

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs1)

    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs2)

    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))

    vectorizer = Vectorizer()
    nom, cont = vectorizer.apply(flat_fs3)

    for i, val in enumerate(nom):
        print(vectorizer.invert(i,val))
    for i, val in enumerate(cont):
        print(vectorizer.invert(i,val))

    print(nom, cont)
# def test_Vectorizer_invert():
#     pass

def test_RelativeEncoder():
    pass

def test_Flattener_incr():
    pass

def test_Vectorizer_incr():
    pass

def test_RelativeEncoder_incr():
    pass


@pytest.mark.benchmark(group="transform")
def test_b_flatten_1000(benchmark):
    pass

@pytest.mark.benchmark(group="transform")
def test_b_vectorize_1000(benchmark):
    pass

@pytest.mark.benchmark(group="transform")
def test_b_relative_encode_1000(benchmark):
    pass

@pytest.mark.benchmark(group="transform")
def test_b_pipeline_1st_run_1000(benchmark):
    pass

@pytest.mark.benchmark(group="transform")
def test_b_pipeline_2nd_run_1000(benchmark):
    pass

@pytest.mark.benchmark(group="transform")
def test_b_pipeline_incr_change(benchmark):
    pass


if __name__ == "__main__":
    # test_Flattener_basic()
    # test_Flattener_refs()
    test_Vectorizer_basic()
    test_Vectorizer_refs()
    # test_Vectorizer_invert()
    # test_RelativeEncoder()
    # test_Flattener_incr()
    # test_Vectorizer_incr()
    # test_RelativeEncoder_incr()
