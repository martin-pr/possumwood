#pragma once

#include <boost/filesystem/path.hpp>

#include "generic_polymesh.h"

namespace possumwood {
namespace polymesh {

GenericPolymesh loadObj(boost::filesystem::path path);

}
}
