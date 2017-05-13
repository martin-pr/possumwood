#pragma once

#include <boost/noncopyable.hpp>

#include <QWidget>

#include "factory.inl"

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

	protected:
		virtual void onFlagsChanged(unsigned flags);

	private:
		unsigned m_flags;
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
		static factory_typed<DERIVED> s_pf;
};

template<typename T, typename DERIVED>
factory_typed<DERIVED> property<T, DERIVED>::s_pf;


}
