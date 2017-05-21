#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>

#include "attr.h"
#include "metadata.h"

namespace dependency_graph {

class Datablock;
class Graph;

class Node : public boost::noncopyable {
	public:
		class Port : public boost::noncopyable {
			public:
				Port(Port&& p);

				const std::string& name() const;
				const Attr::Category category() const;
				const unsigned index() const;

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

			private:
				Port(const std::string& name, unsigned id, Node* parent);

				void setDirty(bool dirty);

				std::string m_name;
				unsigned m_id;
				bool m_dirty;
				Node* m_parent;

				friend class Node;
		};

		const std::string& name() const;

		const Metadata& metadata() const;

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		void setBlindData(const T& value);

		/// blind per-node data, to be used by the client application
		///   to store visual information (e.g., node position, colour...)
		template<typename T>
		const T& blindData() const;

		Port& port(size_t index);
		const Port& port(size_t index) const;
		const size_t portCount() const;

		const Graph& graph() const;
		Graph& graph();

	protected:
		Node(const std::string& name, const Metadata* def, Graph* parent);

		void computeInput(size_t index);
		void computeOutput(size_t index);

		template<typename T>
		const T& get(size_t index) const;

		template<typename T>
		void set(size_t index, const T& value);

		bool inputIsNotConnected(const Port& p) const;

	private:
		void markAsDirty(size_t index);

		std::string m_name;
		Graph* m_parent;

		const Metadata* m_meta;
		Datablock m_data;
		std::unique_ptr<Datablock::BaseData> m_blindData;

		std::vector<Port> m_ports;

		friend class Graph;
		friend class Port;
};

}
