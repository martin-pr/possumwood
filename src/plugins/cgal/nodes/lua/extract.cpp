#include "lua/nodes/extract.h"

#include <lua/datatypes/context.h>
#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/variable.inl>

#include "datatypes/meshes.h"
#include "lua/polyhedron.h"

namespace {

possumwood::NodeImplementation s_impl("lua/extract/mesh",
                                      possumwood::lua::Extract<possumwood::Meshes,
                                                               possumwood::cgal::PolyhedronWrapper,
                                                               possumwood::AttrFlags::kVertical>::init);
}
