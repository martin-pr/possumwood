#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>

namespace possumwood {

typedef CGAL::Simple_cartesian<float> CGALKernel;
typedef CGAL::Polyhedron_3<CGALKernel> CGALPolyhedron;
typedef CGALPolyhedron::Point CGALPoint;

}
