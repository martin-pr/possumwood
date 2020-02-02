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

		/// untyped attribute "get" - has to be accessible from inside compute()
		template<typename T>
		const T& get(const InAttr<void>& attr) const;

		template<typename T>
		bool isDirty(const OutAttr<T>& attr) const;

		template<typename T>
		void set(const InAttr<T>& attr, const T& value);

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

		template<typename T>
		void set(const OutAttr<T>& attr, T&& value);

		/// untyped attribute "set" - has to be accessible from inside compute()
		template<typename T>
		void set(const OutAttr<void>& attr, const T& value);

		/// "transfer" set - get a value from a different port and transfer it here.
		/// This is a bit of a hack - used only for Network input compute().
		/// The port has to be non-const - the call might pull, changing its value.
		void transfer(std::size_t index, Port& p);

		/// untyped attribute type testing.
		/// Only useful for untyped attributes - typed attribute have an explicit T
		template<typename T>
		bool is(const TypedAttr<void>& attr) const;

		const Data& data(std::size_t index) const;

	private:
		NodeBase* m_node;
};

}
