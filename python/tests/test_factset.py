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
    pass

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
    fs = random_cats(10)
    print(fs)

    test_declare_retract()
    test_modify()
    test_retact_error()
    test_iter_get_facts()
    test_indexer()
    test_mem_leaks_basic()
    test_mem_leaks_refs()
    test_long_hash()
    test_from_py()
    test_from_json()
    test_to_json()
