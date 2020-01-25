#pragma once

#include <string>
#include <typeindex>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "attr.h"

namespace dependency_graph {

class NodeBase;

class Port : public boost::noncopyable {
	public:
		Port(Port&& p);
		~Port();

		const std::string& name() const;
		Attr::Category category() const;
		unsigned index() const;
		std::type_index type() const;

		// / returns a fully qualified port name (with all nodes and parents)
		std::string fullName() const;

		/// sets a value on the port.
		/// Marks all downstream values dirty.
		template<typename T>
		void set(const T& value);

		/// gets a value from the port.
		/// Pulls on the inputs and causes recomputation if the value is
		/// marked as dirty.
		template<typename T>
		const T& get();

		/// gets a value from the port.
		/// Pulls on the inputs and causes recomputation if the value is
		/// marked as dirty.
		const BaseData& getData();

		/// sets a value on the port.
		/// Marks all downstream values dirty.
		void setData(const BaseData& val);

		/// returns true if given port is dirty and will require recomputation
		bool isDirty() const;

		/// returns a reference to the parent node
		NodeBase& node();
		/// returns a reference to the parent node
		const NodeBase& node() const;

		/// creates a connection between this port an the port in argument.
		/// The direction is always from *this port to p - only applicable
		/// to output ports with an input port as the argument.
		void connect(Port& p);

		/// disconnects two connected ports
		void disconnect(Port& p);

		/// returns true if this port is connected to anything
		bool isConnected() const;

		/// adds a "value changed" callback - to be used by the UI
		boost::signals2::connection valueCallback(const std::function<void()>& fn);
		/// adds a "flags changed" callback - to be used by the UI
		boost::signals2::connection flagsCallback(const std::function<void()>& fn);

		/// "links" two ports - dirtiness and values of source (this) port are directly
		///   transferred to target port. Usable primarily for subnetworks, but
		///   might have other uses as well (e.g., "referencing"). Can effectively
		///   replace "compute". Only output ports can be linked.
		void linkTo(Port& targetPort);
		bool isLinked() const;
		void unlink();

		const Port& linkedTo() const;
		Port& linkedTo();

	private:
		Port(unsigned id, NodeBase* parent);

		Port(const Port&) = delete;
		Port& operator =(const Port& p) = delete;

		void setDirty(bool dirty);

		NodeBase* m_parent;
		unsigned m_id;
		bool m_dirty;

		Port* m_linkedToPort;
		Port* m_linkedFromPort;

		boost::signals2::signal<void()> m_valueCallbacks, m_flagsCallbacks;

		friend class Node;
		friend class NodeBase;
};

}
