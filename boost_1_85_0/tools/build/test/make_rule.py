#!/usr/bin/env python3

# Copyright 2003 Dave Abrahams
# Copyright 2003, 2006 Vladimir Prus
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or https://www.bfgroup.xyz/b2/LICENSE.txt)

# Test the 'make' rule.

import BoostBuild
import string

t = BoostBuild.Tester(pass_toolset=1)

t.write("jamroot.jam", """\
import feature ;
feature.feature test_feature : : free ;

import toolset ;
toolset.flags creator STRING : <test_feature> ;

actions creator
{
    echo $(STRING) > $(<)
}

make foo.bar : : creator : <test_feature>12345678 ;
""")

t.run_build_system()
t.expect_addition("bin/foo.bar")
t.fail_test(t.read("bin/foo.bar").find("12345678") == -1)


# Regression test. Make sure that if a main target is requested two times, and
# build requests differ only in incidental properties, the main target is
# created only once. The bug was discovered by Kirill Lapshin.
t.write("jamroot.jam", """\
exe a : dir//hello1.cpp ;
exe b : dir//hello1.cpp/<hardcode-dll-paths>true ;
""")

t.write("dir/jamfile.jam", """\
import common ;
make hello1.cpp : hello.cpp : common.copy ;
""")

t.write("dir/hello.cpp", "int main() {}\n")

# Show only action names.
t.run_build_system(["-d1", "-n"])
t.fail_test(t.stdout().count("copy") != 1)

t.cleanup()
