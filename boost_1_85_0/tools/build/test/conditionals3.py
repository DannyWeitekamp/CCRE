#!/usr/bin/env python3

# Copyright 2003 Vladimir Prus
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or https://www.bfgroup.xyz/b2/LICENSE.txt)

# Test that conditional properties work, even if property is free, and value
# includes a colon.

import BoostBuild

t = BoostBuild.Tester(use_test_config=False)

t.write("jamroot.jam", """
exe hello : hello.cpp : <variant>debug:<define>"CLASS=Foo::Bar" ;
""")

t.write("hello.cpp", """
namespace Foo { class Bar { } ; }
int main()
{
    CLASS c;
    c;  // Disables the unused variable warning.
}
""")

t.run_build_system(stdout=None, stderr=None)
t.expect_addition("bin/$toolset/debug*/hello.exe")

t.cleanup()
