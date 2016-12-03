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


				/// returns true if given port is dirty

			private:
				Port(const std::string& name, unsigned id, Node* parent);

				std::string m_name;
				unsigned m_id;
				Node* m_parent;

				friend class Node;
		};

		Port& port(const std::string& name);
		const Port& port(const std::string& name) const;

	protected:
		Node(const std::string& name, const Metadata& def);

	private:
		std::string m_name;
		Metadata m_meta;

		std::vector<Port> m_ports;

		friend class Graph;
};
