#pragma once

#include <functional>

#include <boost/noncopyable.hpp>

#include "attr.h"
#include "metadata.h"

class Datablock;
class Graph;

class Node : public boost::noncopyable {
	public:
		class Port : public boost::noncopyable {
			public:
				Port(Port&& p);

				const std::string& name() const;
				const Attr::Category category() const;

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

			private:
				Port(const std::string& name, unsigned id, Node* parent);

				std::string m_name;
				unsigned m_id;
				bool m_dirty;
				Node* m_parent;

				friend class Node;
		};

		const Metadata& metadata() const;

		Port& port(size_t index);
		const Port& port(size_t index) const;

	protected:
		Node(const std::string& name, const Metadata& def, Graph* parent);

	private:
		std::string m_name;
		Graph* m_parent;

		Metadata m_meta;
		Datablock m_data;

		std::vector<Port> m_ports;

		friend class Graph;
};
