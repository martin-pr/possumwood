#pragma once

#include "datatypes/uniforms.h"

namespace possumwood {

/// returns the viewport uniforms - modelview (straight + normal), projection matrices and resolution
void addViewportUniforms(possumwood::Uniforms& uniforms);

}
