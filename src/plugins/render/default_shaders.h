#pragma once

#include "datatypes/shader.h"

namespace possumwood {

const std::string& defaultVertexShaderSrc();
const possumwood::VertexShader& defaultVertexShader();

const std::string& defaultFragmentShaderSrc();
const possumwood::FragmentShader& defaultFragmentShader();

const std::string& defaultGeometryShaderSrc();
const possumwood::GeometryShader& defaultGeometryShader();

}  // namespace possumwood
