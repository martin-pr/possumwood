#pragma once

#include <boost/noncopyable.hpp>

#include "node.h"

class Graph {
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
				Nodes();
		};

		Nodes& nodes();
		const Nodes& nodes() const;

		struct Connection {
			Node::Port& src, dest;
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
		};

		Connections& connections();
		const Connections& connections() const;
};
