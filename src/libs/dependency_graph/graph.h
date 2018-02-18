#pragma once

#include <vector>
#include <functional>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "node.h"
#include "nodes.h"
#include "connections.h"

namespace dependency_graph {

/// The graph data structure - holds node instances and connections.
class Graph : public boost::noncopyable {
	public:
		Graph();

		bool empty() const;
		void clear();

		Nodes& nodes();
		const Nodes& nodes() const;

		Connections& connections();
		const Connections& connections() const;

		boost::signals2::connection onAddNode(std::function<void(Node&)> callback);
		boost::signals2::connection onRemoveNode(std::function<void(Node&)> callback);

		boost::signals2::connection onConnect(std::function<void(Port&, Port&)> callback);
		boost::signals2::connection onDisconnect(std::function<void(Port&, Port&)> callback);

		boost::signals2::connection onBlindDataChanged(std::function<void(Node&)> callback);
		boost::signals2::connection onNameChanged(std::function<void(Node&)> callback);

		/// dirtiness callback - called when any dirty flag of any port changes (usable for viewport refresh)
		boost::signals2::connection onDirty(std::function<void()> callback);
		/// per-node state change callback
		boost::signals2::connection onStateChanged(std::function<void(const Node&)> callback);

	private:
		void nameChanged(Node& node);
		void stateChanged(Node& node);
		void dirtyChanged();
		void nodeAdded(Node& node);
		void nodeRemoved(Node& node);
		void blindDataChanged(Node& node);

		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

		Nodes m_nodes;
		Connections m_connections;

		boost::signals2::signal<void(Node&)> m_onAddNode, m_onRemoveNode, m_onBlindDataChanged, m_onNameChanged;
		boost::signals2::signal<void(Port&, Port&)> m_onConnect, m_onDisconnect;
		boost::signals2::signal<void()> m_onDirty;
		boost::signals2::signal<void(const Node&)> m_onStateChanged;

		void connected(Port& p1, Port& p2);
		void disconnected(Port& p1, Port& p2);

		friend class Node;
		friend class Nodes;
		friend class Connections;
};

}
