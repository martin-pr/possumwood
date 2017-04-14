## Synopsis

Dependency graph is a way of describing algorithms in terms of inputs, outputs, functions (nodes) and their dependencies. This small library is intended to provide a minimalistic implementation of a Qt-based user interface for editing graph structures.

At the moment, it is in **early stages of development** and as such its APIs might not be complete (and are still subject to change). Please stay tuned :)

[![Build Status](https://travis-ci.org/martin-pr/qt_node_editor.svg?branch=master)](https://travis-ci.org/martin-pr/qt_node_editor)

## Code Example

The library is intended to provide a minimalistic interface for editing graph-like nodal structures with interconnected ports. It does not provide a comprehensive data model, only a minimal interface required for visually representing nodal structures.

For a minimal example, please have a look at the [Demo main.cpp source](https://github.com/martin-pr/qt_node_editor/blob/master/src/main.cpp).

## Motivation

This repository is intended as part of the **Animation sandbox** project (not public yet), to be used in connection with a [Dependency graph evaluation engine](https://github.com/martin-pr/dependency_graph).

## Installation

Not installable for the time being. The project is structured as a standard CMake-built project. To build, just run these in the directory of the repository on any Linux distro:

```
mkdir build
cd build
cmake .. && make -j
```

The code does not use any architecture or OS-specific features or libraries, and it should be compilable without any change on any common OS.

## API Reference

[Doxygen-generated documentation](https://martin-pr.github.io/qt_node_editor) from last passing build.

## Contributors

Please feel free to contribute! I would welcome any form of feedback, opinions, bugs, code, or description of projects you are planning to use my code in!

## License

The code is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).
