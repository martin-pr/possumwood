## Synopsis

Possumwood (named after [Sandbox Tree](https://en.wikipedia.org/wiki/Hura_crepitans)) is a simple graph-based procedural authoring tool, in concept not dissimilar to popular CG packages like Houdini, Blender or Maya.

Its main strength is its **simplicity** - it is trivial to extend it to work with new libraries and data types, and allows free experimentation with parameters of each algorithm.

The core of the project is based on a [Qt-based node editor UI](https://github.com/martin-pr/qt_node_editor) and [Dependency Graph evaluation engine](https://github.com/martin-pr/dependency_graph).

At the moment, it is in **very early stages of development** and as such it is not yet usable. Please stay tuned :)

[![Build Status](https://travis-ci.org/martin-pr/possumwood.svg?branch=master)](https://travis-ci.org/martin-pr/possumwood)

## Motivation

There are many great computer graphics and computational libraries available (e.g., [CGAL](http://www.cgal.org/), [WildMagic](https://www.geometrictools.com/), [Eigen](http://eigen.tuxfamily.org/), [PhysBam](http://physbam.stanford.edu/) ...). While extremely flexible and powerful, experimenting with them requires a lot of effort. At the same time, most professional commercial graphics applications use a proven technique to allow users to approach even very complex procedural algorithms - a graph-based graphical programming language.

This project aims at creating a simple and extensible sandbox for experimentation with graphics libraries and algorithms. While it will probably not reach the heights of applications like Houdini or Fabric Engine, it is intended as a simple and approachable open alternative.

Eventually, it will include a set of plugins containing the bindings for different libraries. At that stage, though, I would use some help :) Feel free to contact me!

## Code Example

(to come)

## Installation

Not installable for the time being. The project is structured as a standard CMake-built project. To build, just run these in the directory of the repository on any Linux distro:

```
mkdir build
cd build
cmake .. && make -j
```

The code does not use any architecture or OS-specific features or libraries, and it should be compilable without any change on any OS.

## API Reference

(to come)

## Contributors

Please feel free to contribute! I would welcome any form of feedback, opinions, bugs, code, or description of projects you are planning to use my code in!

## License

The code is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).
