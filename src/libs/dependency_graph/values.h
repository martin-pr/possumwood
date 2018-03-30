#pragma once

#include <boost/noncopyable.hpp>

#include "node.h"

namespace dependency_graph {

class Values : public boost::noncopyable {
	public:
		Values(NodeBase& n);

		Values(Values&& vals);

		Values& operator =(Values&& vals);

		template<typename T>
		const T& get(const InAttr<T>& attr) const;

		template<typename T>
		const T& get(const OutAttr<T>& attr) const;

		template<typename T>
		bool isDirty(const OutAttr<T>& attr) const;

		template<typename T>
		void set(const InAttr<T>& attr, const T& value);

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

		/// untyped attribute "set" - has to be accessible from inside compute()
		template<typename T>
		void set(const OutAttr<void>& attr, const T& value);

	private:
		NodeBase* m_node;
};

}
