#pragma once

#include <memory>
#include <string>
#include <map>
#include <iostream>

#include <boost/noncopyable.hpp>

namespace properties {

class factory;

class property_base;

template<typename T, typename DERIVED>
class property;

class factories : public boost::noncopyable {
	public:
		static factories& singleton();

		template<typename T>
		std::unique_ptr<property_base> create();

	private:
		factories();

		void add(factory* f);
		void remove(factory* f);

		std::map<std::string, factory*> m_factories;

		template<typename T>
		friend class factory_typed;
};

class factory : public boost::noncopyable {
	public:
		virtual ~factory();

		virtual std::string type() const = 0;
		virtual std::unique_ptr<property_base> create() = 0;

	protected:
		factory();

	private:
};

template<typename T>
class factory_typed : public factory {
	public:
		virtual std::string type() const override;
		virtual std::unique_ptr<property_base> create() override;

	protected:
		factory_typed();
		virtual ~factory_typed();

		template<typename VALUE, typename DERIVED>
		friend class property;
};

}
