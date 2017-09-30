## Synopsis

![Possumwood UI](doc/possumwood_half.gif?raw=true)

Possumwood (named after [Sandbox Tree](https://en.wikipedia.org/wiki/Hura_crepitans)) is a graph-based procedural authoring tool, in concept not dissimilar to popular CG packages like Houdini, Blender or Maya. It is intended to serve as a **sandbox** for computer graphics algorithms and libraries, providing a user-friendly and coding-free UI for libraries that would otherwise be inaccessible for an average user.

It is built on top of a simple graph-evaluation engine, and a Qt-based node graph editor, with an OpenGL viewport. Its main strength is its **extensibility** - it is trivial to implement new plugins supporting new libraries and data types, and allows free user-friendly experimentation with parameters of each algorithm.

At the moment, the project is in its **prototype stage**, and any feedback or help is welcome. In particular, it is in dire need of **a MS Windows build**, preferably using **AppVeyor**. Any help in that area would be greatly appreciated!

[![Build Status](https://travis-ci.org/martin-pr/possumwood.svg?branch=master)](https://travis-ci.org/martin-pr/possumwood)

## Motivation

There are many great computer graphics and computational libraries available (e.g., [CGAL](http://www.cgal.org/), [WildMagic](https://www.geometrictools.com/), [Eigen](http://eigen.tuxfamily.org/), [PhysBam](http://physbam.stanford.edu/) ...). While extremely flexible and powerful, experimenting with them requires a lot of effort. At the same time, most professional commercial graphics applications use a proven technique to allow users to approach even very complex procedural algorithms - a graph-based graphical programming language.

This project aims at creating a simple and extensible sandbox for experimentation with graphics libraries and algorithms. While it will probably not reach the heights of applications like Houdini or Fabric Engine, it is intended as a simple and approachable open alternative.

## Code Example

A simple node, allowing to add two numbers, would consist of a single C++ source file:

```
#include <possumwood_sdk/node_implementation.h>

namespace {

dependency_graph::InAttr<float> input1, input2;
dependency_graph::OutAttr<float> output;

dependency_graph::State compute(dependency_graph::Values& data) {
	const float a = data.get(input1);
	const float b = data.get(input2);

	data.set(output, a + b);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(input1, "a");
	meta.addAttribute(input2, "b");
	meta.addAttribute(output, "out");

	meta.addInfluence(input1, output);
	meta.addInfluence(input2, output);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("maths/add", init);

}
```

## Installation

Not installable for the time being. The project is structured as a standard CMake-built project. To build, just run these in the directory of the repository on any Linux distro:

```
mkdir build
cd build
cmake .. && make -j
```

The code does not use any architecture or OS-specific features or libraries, and it should be compilable without any change on any OS.

## API Reference

An auto-generated Doxygen reference can be found [here](https://martin-pr.github.io/possumwood/annotated.html)

## Contributors

Please feel free to contribute! I would welcome any form of feedback, opinions, bugs, code, or description of projects you are planning to use my code in!

## License

The code is released under the [MIT license](https://en.wikipedia.org/wiki/MIT_License).
