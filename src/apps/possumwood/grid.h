#pragma once

#include <OpenEXR/ImathMatrix.h>
#include <possumwood_sdk/gl_renderable.h>

#include <boost/noncopyable.hpp>

namespace possumwood {

/// A simple drawable grid, compatible with Core GL profile. To be created during a draw
/// call, when a GL context is active.
class Grid : public possumwood::GLRenderable {
  public:
	Grid();
};
}  // namespace possumwood
