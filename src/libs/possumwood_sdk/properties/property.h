#pragma once

#include <actions/actions.h>
#include <dependency_graph/unique_id.h>
#include <possumwood_sdk/app.h>

#include <QWidget>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>
#include <dependency_graph/node_base.inl>
#include <dependency_graph/port.inl>

#include "factory.inl"

namespace possumwood {
namespace properties {

class property_base : public boost::noncopyable {
  public:
	enum {
		kInput = 1,
		kOutput = 2,
		kDisabled = 4,
		// kDirty = 8 // not needed - the widget refreshes itself immediately
	};

	virtual ~property_base() = 0;

	virtual QWidget* widget() = 0;

	unsigned flags() const;
	void setFlags(unsigned flags);

	/// called when the UI value is changed by the user
	boost::signals2::connection valueCallback(std::function<void()> fn);

	/// transfer the typed value to a port
	virtual void valueToPort(dependency_graph::Port& port) const = 0;
	/// transfer the typed value from a port
	virtual void valueFromPort(dependency_graph::Port& port) = 0;

  protected:
	/// can be reimplemented to change the UI's properties based on flag changes
	virtual void onFlagsChanged(unsigned flags);

	/// a callback to be connected to widget's "onChange" signal
	void callValueChangedCallbacks();

  private:
	unsigned m_flags;
	boost::signals2::signal<void()> m_valueChanged;
};

template <typename T, typename DERIVED>
class property;

template <typename T, typename DERIVED, typename ENABLE = void>
struct property_updater {};

template <typename T, typename DERIVED>
struct property_updater<T, DERIVED, typename std::enable_if<std::is_copy_constructible<T>::value>::type> {
	static void update(dependency_graph::Port& port, const property<T, DERIVED>& prop) {
		// get the current value
		T value = port.get<T>();
		// allow the UI to change it (or a part of it)
		prop.get(value);
		// and use action to apply it to a port (making it undoable)
		possumwood::actions::setValue(port, std::move(value));
	}
};

template <typename T, typename DERIVED>
struct property_updater<T, DERIVED, typename std::enable_if<not std::is_copy_constructible<T>::value>::type> {
	static void update(dependency_graph::Port& port, const property<T, DERIVED>& prop) {
		assert(false && "property::valueToPort() cannot be used on noncopyable types");
	}
};

template <typename T, typename DERIVED>
class property : public property_base {
  public:
	typedef T result_type;
	typedef DERIVED derived_type;

	property() : m_blockedSignals(false) {
	}

	virtual ~property() {
		// make sure the factory is not optimized away
		static factory_typed<DERIVED>* dummy;
		dummy = &s_pf;
		dummy->type();
	}

	virtual void get(T& value) const = 0;
	virtual void set(const T& value) = 0;

  private:
	virtual void valueToPort(dependency_graph::Port& port) const override final {
		// transfer the templated value
		if((flags() & kInput) && !(flags() & kDisabled) && !m_blockedSignals)
			property_updater<T, DERIVED>::update(port, *this);
	}

	void valueFromPort(dependency_graph::Port& port) override final {
		// transfer the templated value
		if(!m_blockedSignals && port.type() == typeid(T)) {
			bool block = widget()->blockSignals(true);
			m_blockedSignals = true;
			set(port.get<T>());
			widget()->blockSignals(block);
			m_blockedSignals = false;
		}
	}

	bool m_blockedSignals;

	static factory_typed<DERIVED> s_pf;
};

template <typename T, typename DERIVED>
factory_typed<DERIVED> property<T, DERIVED>::s_pf;

}  // namespace properties
}  // namespace possumwood
