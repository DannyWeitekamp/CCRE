import os, glob, importlib.util

# Monkey-Patch import dummy_ext for testing purposes
build_path = os.path.join(os.path.split(__file__)[0], "../../build/")
ext_path = glob.glob(os.path.normpath(build_path) + "/dummy_ext.*")[0]
spec = importlib.util.spec_from_file_location("dummy_ext", ext_path)
module = importlib.util.module_from_spec(spec)

from dummy_ext import Dog, Kennel



d = Dog("dingle")
d.name = "woofy"
print(d.get_refcount())
d = None

print("^ 'woofy' Shoulda died.")

print()

k = Kennel("scratchy", 50)
print("kennel:", k.get_refcount())
k = None
print("^  Kennel Shoulda died.")
print("^ 'scratchy' Shoulda died.")

print()

k = Kennel("scrubs", 64)
print('k->scrubs (1)=2,', k.dog.get_refcount())
print('k->scrubs (2)=2,', k.dog.get_refcount())
print('k->scrubs (3)=2,', k.dog.get_refcount())
print("kennel:", k.get_refcount())
d = k.dog
print('k->scrubs (4)=3,', k.dog.get_refcount())
k = None
print("^  Kennel Shoulda died.")
print('k->scrubs (5)=2,', d.get_refcount())
d = None
print("^ 'scrubs' Shoulda died.")

print("\n--- End of Program --\n")
