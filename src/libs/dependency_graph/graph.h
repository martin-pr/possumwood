#pragma once

#include <vector>
#include <functional>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "node.h"
#include "nodes.h"
#include "connections.h"
#include "network.h"

namespace dependency_graph {

/// The graph data structure - holds node instances and connections.
class Graph : public Network {
	public:
		Graph();
		~Graph();

		boost::signals2::connection onAddNode(std::function<void(Node&)> callback);
		boost::signals2::connection onRemoveNode(std::function<void(Node&)> callback);

		boost::signals2::connection onConnect(std::function<void(Port&, Port&)> callback);
		boost::signals2::connection onDisconnect(std::function<void(Port&, Port&)> callback);

		boost::signals2::connection onBlindDataChanged(std::function<void(NodeBase&)> callback);
		boost::signals2::connection onNameChanged(std::function<void(NodeBase&)> callback);

		/// dirtiness callback - called when any dirty flag of any port changes (usable for viewport refresh)
		boost::signals2::connection onDirty(std::function<void()> callback);
		/// per-node state change callback
		boost::signals2::connection onStateChanged(std::function<void(const Node&)> callback);

	private:
		void nameChanged(NodeBase& node);
		void stateChanged(Node& node);
		void dirtyChanged();
		void nodeAdded(Node& node);
		void nodeRemoved(Node& node);
		void blindDataChanged(NodeBase& node);

		void connected(Port& p1, Port& p2);
		void disconnected(Port& p1, Port& p2);

		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

		struct Signals;
		std::unique_ptr<Signals> m_signals;

		friend class NodeBase;
		friend class Node;
		friend class Nodes;
		friend class Connections;
};

}
