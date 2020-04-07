#pragma once

#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <typeindex>

#include <boost/noncopyable.hpp>

namespace possumwood { namespace properties {

class factory;

class property_base;

template<typename T, typename DERIVED>
class property;

class factories : public boost::noncopyable {
	public:
		static factories& singleton();

		std::unique_ptr<property_base> create(const std::type_index& type);

	private:
		factories();

		void add(factory* f);
		void remove(factory* f);

		std::map<std::type_index, factory*> m_factories;

		template<typename T>
		friend class factory_typed;
};

class factory : public boost::noncopyable {
	public:
		virtual ~factory();

		virtual std::type_index type() const = 0;
		virtual std::type_index uiType() const = 0;
		virtual std::unique_ptr<property_base> create() = 0;

	protected:
		factory();

	private:
};

template<typename T>
class factory_typed : public factory {
	public:
		virtual std::type_index type() const override;
		virtual std::type_index uiType() const override;
		virtual std::unique_ptr<property_base> create() override;

	protected:
		factory_typed();
		virtual ~factory_typed();

		template<typename VALUE, typename DERIVED>
		friend class property;

		std::type_index m_type;
};

} }
