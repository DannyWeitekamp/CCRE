#!/usr/bin/env python3

# Copyright 2012 Steven Watanabe
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or https://www.bfgroup.xyz/b2/LICENSE.txt)

# This tests the EXIT rule.

import BoostBuild

def test_exit(name):
    t = BoostBuild.Tester(["-ffile.jam"], pass_toolset=0)

    t.write("file.jam", "%s ;" % name)
    t.run_build_system(status=1, stdout="\n")
    t.rm(".")

    t.write("file.jam", "%s : 0 ;" % name)
    t.run_build_system(stdout="\n")
    t.rm(".")

    t.write("file.jam", "%s : 1 ;" % name)
    t.run_build_system(status=1, stdout="\n")
    t.rm(".")

    t.write("file.jam", "%s : 2 ;" % name)
    t.run_build_system(status=2, stdout="\n")
    t.rm(".")

    t.write("file.jam", "%s a message ;" % name)
    t.run_build_system(status=1, stdout="a message\n")
    t.rm(".")

    t.write("file.jam", "%s a message : 0 ;" % name)
    t.run_build_system(stdout="a message\n")
    t.rm(".")

    t.cleanup()

test_exit("EXIT")
test_exit("Exit")
test_exit("exit")
