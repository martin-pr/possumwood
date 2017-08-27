#pragma once

#include <boost/noncopyable.hpp>

#include "node.h"

namespace dependency_graph {

class Values : public boost::noncopyable {
	public:
		Values(Node& n);

		Values(Values&& vals);

		Values& operator =(Values&& vals);

		template<typename T>
		bool isConnected(const InAttr<T>& attr) const;

		template<typename T>
		const T& get(const InAttr<T>& attr) const;

		template<typename T>
		const T& get(const OutAttr<T>& attr) const;

		template<typename T>
		void set(const InAttr<T>& attr, const T& value);

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

	private:
		Node* m_node;
};

}
