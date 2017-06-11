#pragma once

#include <functional>
#include <string>

#include <boost/noncopyable.hpp>

#include "metadata.h"

namespace possumwood {

/// a simple instantiation object, to be used to register a new node type in a cpp file
struct NodeImplementation : public boost::noncopyable {
	public:
		NodeImplementation(const std::string& nodeName, std::function<void(Metadata&)> init);

	private:
		Metadata m_meta;
};

}
