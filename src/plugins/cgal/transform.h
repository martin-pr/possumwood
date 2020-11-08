#pragma once

#include <OpenEXR/ImathMatrix.h>

#include "meshes.h"

namespace possumwood {

Meshes transform(const Meshes& meshes, const Imath::Matrix44<float>& tr, bool transformVec3PropsAsNormals);

}
