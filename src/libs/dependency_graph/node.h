#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "node_base.h"
#include "attr.h"
#include "metadata.h"
#include "port.h"

namespace dependency_graph {

class Node : public NodeBase {
	public:
		virtual ~Node();

	protected:
		Node(const std::string& name, const UniqueId& id, const MetadataHandle& def, Network* parent);

	private:

	friend class Network;
};

}
