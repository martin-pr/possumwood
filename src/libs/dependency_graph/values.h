#pragma once

#include <boost/noncopyable.hpp>

#include "node.h"

namespace dependency_graph {

class Values : public boost::noncopyable {
	public:
		Values(Node& n);

		template<typename T>
		const T& get(const InAttr<T>& attr) const;

		template<typename T>
		const T& get(const OutAttr<T>& attr) const;

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

	private:
		Node* m_node;
};

}
