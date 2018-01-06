#pragma once

#include <OpenEXR/ImathVec.h>

#include <CGAL/Simple_cartesian.h>
// #include <CGAL/Surface_mesh.h>
#include <CGAL/Polyhedron_3.h>

namespace possumwood {

typedef CGAL::Simple_cartesian<float> CGALKernel;
// typedef CGAL::Surface_mesh<CGALKernel::Point_3> CGALPolyhedron;
typedef CGAL::Polyhedron_3<CGALKernel> CGALPolyhedron;

}
