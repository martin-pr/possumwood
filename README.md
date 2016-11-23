## Synopsis

Dependency graph is a way of describing algorithms in terms of inputs, outputs, functions (nodes) and their dependencies. This small library is intended to provide a minimalistic implementation of an evaluation engine for such a graph, based on C++ template metaprogramming and TBB.

At the moment, it is in **very early stages of development** and as such it is not yet usable. Please stay tuned :)

[![Build Status](https://travis-ci.org/martin-pr/dependency_graph.svg?branch=master)](https://travis-ci.org/martin-pr/dependency_graph)

## Code Example

The main idea is to provide a minimalistic interface to define metadata of functions (in terms of their inputs and outputs) and a mechanism to instantiate and connect these to create new algorithms. This library does not provide a full implementation of a node graph (e.g., there is no concept of new node registration and factories to create nodes), only the raw evaluation engine itself, which can be abstracted further (a simple fully-fledged node framework might come in the future as a separate module).

For a minimal example, please have a look at the [Arithmetic test/example](https://github.com/martin-pr/dependency_graph/blob/master/tests/arithmetic.cpp).

## Motivation

This repository is intended as part of the **Animation sandbox** project (not public yet), to be used in connection with a [Qt-based node editor](https://github.com/martin-pr/qt_node_editor).

## Installation

Not installable for the time being. The project is structured as a standard CMake-built project. To build, just run these in the directory of the repository on any Linux distro:

```
mkdir build
cd build
cmake .. && make -j
```

The code does not use any architecture or OS-specific features or libraries, and it should be compilable without any change on any OS.

## API Reference

[Doxygen-generated documentation](https://martin-pr.github.io/dependency_graph) from last passing build.

## Contributors

Please feel free to contribute! I would welcome any form of feedback, opinions, bugs, code, or description of projects you are planning to use my code in!

## License

The code is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).
