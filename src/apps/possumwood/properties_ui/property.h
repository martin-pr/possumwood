#pragma once

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include <QWidget>

#include "factory.inl"
#include "node.inl"

namespace properties {

class property_base : public boost::noncopyable {
	public:
		enum {
			kInput = 1,
			kOutput = 2,
			kDisabled = 4,
			kDirty = 8
		};

		virtual ~property_base() = 0;

		virtual QWidget* widget() = 0;

		unsigned flags() const;
		void setFlags(unsigned flags);

		/// called when the UI value is changed by the user
		boost::signals2::connection valueCallback(std::function<void()> fn);

		/// transfer the typed value to a port
		virtual void valueToPort(dependency_graph::Node::Port& port) const = 0;
		/// transfer the typed value from a port
		virtual void valueFromPort(dependency_graph::Node::Port& port) = 0;

	protected:
		/// can be reimplemented to change the UI's properties based on flag changes
		virtual void onFlagsChanged(unsigned flags);

		/// a callback to be connected to widget's "onChange" signal
		void callValueChangedCallbacks();

	private:
		unsigned m_flags;
		boost::signals2::signal<void()> m_valueChanged;
};

template<typename T, typename DERIVED>
class property : public property_base {
	public:
		typedef T result_type;
		typedef DERIVED derived_type;

		virtual ~property() {
			// make sure the factory is not optimized away
			static factory_typed<DERIVED>* dummy;
			dummy = &s_pf;
			dummy->type();
		}

		virtual T get() const = 0;
		virtual void set(const T& value) = 0;

	private:
		virtual void valueToPort(dependency_graph::Node::Port& port) const override {
			// transfer the templated value
			port.set(get());
		}

		void valueFromPort(dependency_graph::Node::Port& port) override {
			// transfer the templated value
			set(port.get<T>());
		}

		static factory_typed<DERIVED> s_pf;
};

template<typename T, typename DERIVED>
factory_typed<DERIVED> property<T, DERIVED>::s_pf;


}
