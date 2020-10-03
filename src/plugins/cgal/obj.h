#pragma once

#include <boost/filesystem/path.hpp>

#include "mesh.h"

namespace possumwood {

Mesh loadObj(boost::filesystem::path path, const std::string& name, const std::string& normalsAttr = "N");

}
