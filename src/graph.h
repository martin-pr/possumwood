#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include "node.h"

class Graph : public boost::noncopyable {
	public:
		Graph();

		class Nodes : public boost::noncopyable {
			public:
				Node& add(const Metadata& type, const std::string& name);
				// std::size_t size() const;

				// typedef ... const_iterator;
				// const_iterator begin() const;
				// const_iterator end() const;

				// typedef ... iterator;
				// iterator begin();
				// iterator end();

				// iterator erase(iterator i);

			private:
				Nodes() = default;

				// stored in a pointer container, to keep parent pointers
				//   stable without too much effort (might change)
				std::vector<std::unique_ptr<Node>> m_nodes;

				friend class Graph;
		};

		Nodes& nodes();
		const Nodes& nodes() const;

		struct Connection {
			const Node::Port* src;
			const Node::Port* dest;

			bool operator ==(const Connection& c) {
				return src == c.src && dest == c.dest;
			}
		};

		class Connections : public boost::noncopyable {
			public:
				Connection& add(const Node::Port& src, const Node::Port& dest);
				// std::size_t size() const;

				// typedef ... const_iterator;
				// const_iterator begin() const;
				// const_iterator end() const;

				// typedef ... iterator;
				// iterator begin();
				// iterator end();

				// iterator erase(iterator i);

			private:
				Connections() = default;

				std::vector<Connection> m_connections;

				friend class Graph;
		};

		Connections& connections();
		const Connections& connections() const;

	private:
		static std::unique_ptr<Node> makeNode(const std::string& name, const Metadata& md);

		Nodes m_nodes;
		Connections m_connections;

};
