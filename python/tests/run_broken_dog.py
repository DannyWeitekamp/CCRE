import os, glob, importlib.util

# Monkey-Patch import dummy_ext for testing purposes
build_path = os.path.join(os.path.split(__file__)[0], "../../build/")
ext_path = glob.glob(os.path.normpath(build_path) + "/dummer_ext.*")[0]
spec = importlib.util.spec_from_file_location("dummer_ext", ext_path)
module = importlib.util.module_from_spec(spec)

from dummer_ext import Dog, Kennel, leaked_objects
d = Dog("woofy")
d = None
print("^ 'woofy' Shoulda died.\n")

k = Kennel("scratchy", 50)
# print("kennel:", k.get_refcount())
k = None
print("^  Kennel Shoulda died.")
print("^ 'scratchy' Shoulda died.\n")

k = Kennel("buddy", 64)
# print('k->buddy (1)=2,', k.dog.get_refcount())
# print('k->buddy (2)=2,', k.dog.get_refcount())
# print('k->buddy (3)=2,', k.dog.get_refcount())
# print("kennel:", k.get_refcount())
d = k.dog
print("k.dog is k.dog: ", d is k.dog)
# print('k->buddy (4)=3,', k.dog.get_refcount())
k = None
print("^  Kennel Shoulda died.")
# print('k->buddy (5)=2,', d.get_refcount())
d = None
print("^ 'buddy' Shoulda died.")

print("\n--- End of Program --")
print("Leaked Objects: ", leaked_objects())
