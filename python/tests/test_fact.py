from test_utils import *
import pytest



def test_define_fact():
    from cre import define_fact

    Cat = define_fact("Cat", 
        {"id" : int,
         "name" : str,
         "color" : str,
         "legs" : int,
         "frisky" : bool,
         "x" : float,
         "y" : float
        }
    );


    Cat = define_fact("Cat", 
        {"id" : "int",
         "name" : "str",
         "color" : "str",
         "legs" : "int",
         "frisky" : "bool",
         "x" : "float",
         "y" : "float"
        }
    );

    from cre import CRE_Context
    curr_context = CRE_Context()
    print(curr_context)

    # TODO: w/ flags


def test_define_fact_deffered():
    from cre import define_fact, DefferedType

    #-- Test Auto self-resolution --
    Person = define_fact("Person", 
        {"id" : str,
         "friend" : "Person",
         "mother" : "Person",
        }
    );

    # -- Test Explicit Usage --
    Dog = DefferedType("Dog")
    DogOwner = define_fact("DogOwner", 
        {"id" : str,
         "dog" : Dog,
        }
    )

    # Cannot instantiate types with unresolved members 
    with pytest.raises(Exception):
        pete = DogOwner("Pete")

    Dog = define_fact("Dog", 
        {"id" : str,
         "owner" : DogOwner,
        }
    )

    pete = DogOwner("Pete")
    Dog("fido", pete)



    # -- Test Auto Differed --
    CrabOwner = define_fact("CrabOwner", 
        {"id" : str,
         "crab" : "Crab",
        }
    )

    # Cannot instantiate types with unresolved members 
    with pytest.raises(Exception):
        bobby = CrabOwner("Bobby")

    Crab = define_fact("Crab", 
        {"id" : str,
         "owner" : CrabOwner,
        }
    )

    bobby = CrabOwner("Bobby")
    Crab("pinchy", bobby)    




def test_fact_create():
    from cre import define_fact, NewFact

    # Test Typed Facts
    Cat = define_fact("Cat", 
        {"name" : str,
         "color" : str,
         "legs" : int,
         "frisky" : bool,
         "x" : float,
         "y" : float
        }
    );

    snowball = Cat("snowball", "white", 3, False, 1.0, 1.0)

    with pytest.raises(Exception):
        Cat("snowball", 77, 3, False, 1.0, 1.0)

    with pytest.raises(Exception):
        Cat("snowball", "white", 3, "blue", 1.0, 1.0)

    return snowball

    

def test_fact_getitem():
    snowball = test_fact_create()    
    
    # Test __getitem__
    assert snowball[0] == "snowball"
    assert snowball[1] == "white"
    assert snowball[2] == 3
    assert snowball[3] == False
    assert snowball[4] == 1.0
    assert snowball[5] == 1.0
    assert snowball[-3] == False
    assert snowball[-6] == "snowball"

    with pytest.raises(Exception):
        snowball[6]

    with pytest.raises(Exception):
        snowball[-7]

def test_fact_getattr():
    snowball = test_fact_create()    
    
    # Test __getattr__
    assert snowball["name"] == "snowball"
    assert snowball["color"] == "white"
    assert snowball["legs"] == 3
    assert snowball["frisky"] == False
    assert snowball["x"] == 1.0
    assert snowball["y"] == 1.0

    with pytest.raises(Exception):
        snowball["funhat"]

    assert snowball.name == "snowball"
    assert snowball.color == "white"
    assert snowball.legs == 3
    assert snowball.frisky == False
    assert snowball.x == 1.0
    assert snowball.y == 1.0

    with pytest.raises(Exception):
        snowball.funhat

def test_fact_iter():
    snowball = test_fact_create()    

    # Test __iter__
    for i, item in enumerate(snowball):
        pass

    assert tuple(x for x in snowball) == ("snowball", "white", 3, False, 1.0, 1.0)


def test_fact_eq():
    pass

def test_fact_hash():
    pass

def test_fact_copy():
    pass

def test_fact_slice():
    pass

def test_fact_str():
    pass

def test_list_member():
    pass

def test_protected_mutability():
    pass

def test_as_conditions():
    pass

@pytest.mark.benchmark(group="fact")
def test_b_make_Cat_from_py_1000(benchmark):
    pass

@pytest.mark.benchmark(group="fact")
def test_b_make_Cat_like_dict_1000(benchmark):
    pass

@pytest.mark.benchmark(group="fact")
def test_b_make_PyCat_1000(benchmark):
    pass


if __name__ == "__main__":
    import faulthandler; faulthandler.enable()
    test_define_fact()
    test_define_fact_deffered()
    test_fact_create()
    test_fact_getitem()
    test_fact_getattr()
    
    # test_fact_eq()
    # test_fact_hash()
    # test_fact_copy()
    # test_fact_slice()
    # test_fact_str()
    # test_list_member()
    # test_protected_mutability()
    # test_as_conditions()
