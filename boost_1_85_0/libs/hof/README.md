# Boost.Hof <a target="_blank" href="https://travis-ci.org/boostorg/hof">![Travis status][badge.Travis]</a> <a target="_blank" href="https://ci.appveyor.com/project/pfultz2/hof">![Appveyor status][badge.Appveyor]</a>

About
=====

HigherOrderFunctions is a header-only C++11/C++14 library that provides utilities for functions and function objects, which can solve many problems with much simpler constructs than whats traditionally been done with metaprogramming.

HigherOrderFunctions is:

- Modern: HigherOrderFunctions takes advantages of modern C++11/C++14 features. It support both `constexpr` initialization and `constexpr` evaluation of functions. It takes advantage of type deduction, variadic templates, and perfect forwarding to provide a simple and modern interface. 
- Relevant: HigherOrderFunctions provides utilities for functions and does not try to implement a functional language in C++. As such, HigherOrderFunctions solves many problems relevant to C++ programmers, including initialization of function objects and lambdas, overloading with ordering, improved return type deduction, and much more.
- Lightweight: HigherOrderFunctions builds simple lightweight abstraction on top of function objects. It does not require subscribing to an entire framework. Just use the parts you need.

HigherOrderFunctions is divided into three components:

* Function Adaptors and Decorators: These enhance functions with additional capability.
* Functions: These return functions that achieve a specific purpose.
* Utilities: These are general utilities that are useful when defining or using functions

Github: [https://github.com/boostorg/hof/](https://github.com/boostorg/hof/)

Documentation: [http://boost-hof.readthedocs.io/](http://boost-hof.readthedocs.io/)

Motivation
==========

- Improve the expressiveness and capabilities of functions, including first-class citizens for function overload set, extension methods, infix operators and much more.
- Simplify constructs in C++ that have generally required metaprogramming
- Enable point-free style programming
- Workaround the limitations of lambdas in C++14

Requirements
============

This requires a C++11 compiler. There are no third-party dependencies. This has been tested on clang 3.5-3.8, gcc 4.6-7, and Visual Studio 2015 and 2017.

Contexpr support
----------------

Both MSVC and gcc 4.6 have limited constexpr support due to many bugs in the implementation of constexpr. However, constexpr initialization of functions is supported when using the [`BOOST_HOF_STATIC_FUNCTION`](BOOST_HOF_STATIC_FUNCTION) and [`BOOST_HOF_STATIC_LAMBDA_FUNCTION`](BOOST_HOF_STATIC_LAMBDA_FUNCTION) constructs.

Noexcept support
----------------

On older compilers such as gcc 4.6 and gcc 4.7, `noexcept` is not used due to many bugs in the implementation. Also, most compilers don't support deducing `noexcept` with member function pointers. Only newer versions of gcc(4.9 and later) support this.

Building
========

Boost.HigherOrderFunctions library uses cmake to build. To configure with cmake create a build directory, and run cmake:

    mkdir build
    cd build
    cmake ..

Installing
----------

To install the library just run the `install` target:

    cmake --build . --target install

Tests
-----

The tests can be built and run by using the `check` target:

    cmake --build . --target check

The tests can also be ran using Boost.Build, just copy library to the boost source tree, and then:

    cd test
    b2

Documentation
-------------

The documentation is built using Sphinx. First, install the requirements needed for the documentation using `pip`:

    pip install -r doc/requirements.txt

Then html documentation can be generated using `sphinx-build`:

    sphinx-build -b html doc/ doc/html/

The final docs will be in the `doc/html` folder.

<!-- Links -->
[badge.Travis]: https://travis-ci.org/boostorg/hof.svg?branch=master
[badge.Appveyor]: https://ci.appveyor.com/api/projects/status/bjj27h3v3bxbgpsp/branch/develop
