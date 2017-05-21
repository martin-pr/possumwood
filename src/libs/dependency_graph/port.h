#pragma once

#include <string>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include "attr.h"

namespace dependency_graph {

class Node;

class Port : public boost::noncopyable {
	public:
		Port(Port&& p);

		const std::string& name() const;
		const Attr::Category category() const;
		const unsigned index() const;
		const std::string type() const;

		/// sets a value on the port.
		/// Marks all downstream values dirty.
		template<typename T>
		void set(const T& value);

		/// gets a value from the port.
		/// Pulls on the inputs and causes recomputation if the value is
		/// marked as dirty.
		template<typename T>
		const T& get();

		/// returns true if given port is dirty and will require recomputation
		bool isDirty() const;

		/// returns a reference to the parent node
		Node& node();
		/// returns a reference to the parent node
		const Node& node() const;

		/// creates a connection between this port an the port in argument.
		/// The direction is always from *this port to p - only applicable
		/// to output ports with an input port as the argument.
		void connect(Port& p);

		/// disconnects two connected ports
		void disconnect(Port& p);

		/// adds a "value changed" callback - to be used by the UI
		boost::signals2::connection valueCallback(const std::function<void()>& fn);
		/// adds a "flags changed" callback - to be used by the UI
		boost::signals2::connection flagsCallback(const std::function<void()>& fn);

	private:
		Port(const std::string& name, unsigned id, Node* parent);

		void setDirty(bool dirty);

		std::string m_name;
		unsigned m_id;
		bool m_dirty;
		Node* m_parent;
		boost::signals2::signal<void()> m_valueCallbacks, m_flagsCallbacks;

		friend class Node;
};

}
