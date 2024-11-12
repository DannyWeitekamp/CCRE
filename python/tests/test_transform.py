from test_utils import *
import pytest

def test_Flattener():
    from cre import Flattener

    CatType = define_fact("_Cat", 
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
    fs.declare(CatType(0, "fluffer", "grey", 4, False, 3.0, 4.0))
    fs.declare(CatType(1, "soren", "white", 4, True, 12.0, 9.0))
    fs.declare(CatType(2, "sango", "brown", 4, True, 13.0, 8.0))
    fs.declare(CatType(3, "crabcake", "orange", 4, False, 3.0, 7.0))

    flattener = Flattener()
    flat_fs = flattener.apply(fs)
    assert len(flat_fs) == 6*len(fs)

    flattener = Flattener(subj_as_fact=True)
    flat_fs = flattener.apply(fs)
    assert len(flat_fs) == 7*len(fs)

    # Refcount sanity check (flat_fs has one, fact has one)
    for fact in flat_fs:
        assert fact.get_refcount() == 2;


def test_Vectorizer():
    pass

def test_Vectorizer_invert():
    pass

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
    test_Flattener()
    test_Vectorizer()
    test_Vectorizer_invert()
    test_RelativeEncoder()
    test_Flattener_incr()
    test_Vectorizer_incr()
    test_RelativeEncoder_incr()
