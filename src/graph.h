#pragma once

#include <boost/noncopyable.hpp>

class Graph {
	public:
		Graph();

		class Nodes : public boost::noncopyable {
			public:
				Node& add(const NodeType& type, const std::string& name, const BlindData& blindData = BlindData());
				std::size_t size() const;

				typedef ... const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				typedef ... iterator;
				iterator begin();
				iterator end();

				iterator erase(iterator i);

			private:
				Nodes();
		};

		Nodes& nodes();
		const Nodes& nodes() const;

		class Connections : public boost::noncopyable {
			public:
				Connection& add(const Node::Attr& src, const Node::Attr& dest);
				std::size_t size() const;

				typedef ... const_iterator;
				const_iterator begin() const;
				const_iterator end() const;

				typedef ... iterator;
				iterator begin();
				iterator end();

				iterator erase(iterator i);
		}

		Connections& connections();
		const Connections& connections() const;
};
