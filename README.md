# ctest

`ctest` is a portable unit-testing framework for C/C++.

It was inspired by several other unit test frameworks, including those
for other languages like Java, that do not appear to exist in a single
framework for C. Most notably:

* [check](http://libcheck.github.io/check/)
* [CppUnit](http://cppunit.sourceforge.net/doc/cvs/index.html)
* [TestNG](http://testng.org/doc/index.html)

It was designed and tested on macOS (10.12) and Linux (CentOS 7), but should
integrate well on other systems (although some tweaks may be required).

## Features

Many of the features were inspired by other testing frameworks for C/C++
and Java.

* Tools for writing clear and concise tests, simplifying the creation of
  new tests.
* Test execution isolation, to ensure the results of one test cannot
  affect the results of another.
* Data providers, to minimize code duplication by writing a single test that is
  run multiple times with different data.
* Test fixtures, to automatically set up execution environment before
  tests are run.
* Editor friendly. The macros for defining/registering test cases won't cause
  your editor to get confused.

## Goals

One of the main goals of `ctest` is to make writing simple tests extremely
simple, with very low (programmer) overhead. At the same time, making it
possible to write much more complex tests, without the programmer
having to jump through too many hoops. In all cases, the amount of
boilerplate code must be kept to a minimum.

> Simple things should be simple. Complex things should be possible.
> — Alan Kay

As such, common idioms used in simple, well-written unit tests are often very
simple to achieve in `ctest`. More advanced features are often significantly
more verbose, but are at least possible. If a test operation is awkward or
complicated to represent using `ctest`, that might be a sign that there is
a simpler, clearer way to achieve the same ends.

## Requirements

`ctest` aims to be portable and does not rely on external dependencies beyond a
modern(ish) C compiler. However, it does depend on support for
[ltdl](https://www.gnu.org/software/libtool) and relies on a few POSIX feature.
Development is primarily on MAC OS X and Linux, so if you experience issues and
would like to use `ctest` on your system,
[file an issue](https://github.com/mkralka/ctest/issues/new).

## Installation

`ctest` is built and installed using autotools (automake, autoconf, and
libtool). After unpacking the source, installing should be as simple as:

```
./configure &&
make &&
sudo make install
```

## What's next?

1. [Tutorial](docs/md/tutorial.md)
2. [API Documentation](docs/md/api.md)
3. [FAQ](docs/md/faq.md)

