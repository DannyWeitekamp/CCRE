#!/usr/bin/env python3

# Copyright 2012. Jurko Gospodnetic
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at
# https://www.bfgroup.xyz/b2/LICENSE.txt)

# Test correct "-p" option handling.

import BoostBuild

t = BoostBuild.Tester(["-d1"], pass_toolset=False)

t.write("file.jam", """\
prefix = "echo \\"" ;
suffix = "\\"" ;
if $(NT)
{
    prefix = "(echo " ;
    suffix = ")" ;
}
actions go
{
    $(prefix)stdout$(suffix)
    >&2 $(prefix)stderr$(suffix)
}
ECHO "{{{" $(XXX) "}}}" ;
ALWAYS all ;
go all ;
""")

t.run_build_system(["-ffile.jam", "-sXXX=1"], stderr="")
t.expect_output_lines("{{{ 1 }}}")
t.expect_output_lines("stdout")
t.expect_output_lines("stderr")
t.expect_nothing_more()

t.run_build_system(["-ffile.jam", "-sXXX=2", "-p0"], stderr="")
t.expect_output_lines("{{{ 2 }}}")
t.expect_output_lines("stdout")
t.expect_output_lines("stderr")
t.expect_nothing_more()

t.run_build_system(["-ffile.jam", "-sXXX=3", "-p1"], stderr="")
t.expect_output_lines("{{{ 3 }}}")
t.expect_output_lines("stdout")
t.expect_output_lines("stderr*", False)
t.expect_nothing_more()

t.run_build_system(["-ffile.jam", "-sXXX=4", "-p2"], stderr="stderr\n")
t.expect_output_lines("{{{ 4 }}}")
t.expect_output_lines("stdout*", False)
t.expect_output_lines("stderr*", False)
t.expect_nothing_more()

t.run_build_system(["-ffile.jam", "-sXXX=5", "-p3"], stderr="stderr\n")
t.expect_output_lines("{{{ 5 }}}")
t.expect_output_lines("stdout")
t.expect_output_lines("stderr*", False)
t.expect_nothing_more()

t.cleanup()
