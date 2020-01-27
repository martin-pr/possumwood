#pragma once

#include <memory>
#include <functional>
#include <type_traits>
#include <map>
#include <sstream>
#include <cassert>

#include <boost/noncopyable.hpp>

#include "data_traits.h"

namespace dependency_graph {

template<typename T>
struct Factory {
	Factory();
};

struct Holder {
	virtual ~Holder() {};

	virtual const std::type_info& typeinfo() const = 0;
	virtual bool isEqual(const Holder& src) const = 0;
	virtual std::string toString() const = 0;
};

template<typename T>
class TypedHolder : public Holder {
	public:
		TypedHolder(const T& d);
		virtual ~TypedHolder();

		virtual const std::type_info& typeinfo() const override;
		virtual bool isEqual(const Holder& src) const override;
		virtual std::string toString() const override;

		T data;
		static Factory<T> m_factory;
};

template<typename T>
struct Maker {
	static std::shared_ptr<const Holder> make(const T& val) {
		return std::shared_ptr<const Holder>(new TypedHolder<T>(val));
	}
};

template<>
struct Maker<decltype(nullptr)> {
};

class Data {
	public:
		/// creates a new Data instance based on type name. Internally implements a factory mechanism to produce instances based on named types.
		static Data create(const std::string& type);

		// creates empty data holder(null data with void type)
		Data();

		// creates a data instance of a particular type initialised to a value
		template<typename T>
		Data(const T& value) : m_data(Maker<T>::make(value)) {
		}

		Data(const Data& d);
		Data& operator = (const Data& d);

		/// returns true if this instance doesn't contain any data (i.e., doesn't have a type)
		bool empty() const;

		std::string type() const;
		const std::type_info& typeinfo() const;

		template<typename T>
		const T& get() const;

		template<typename T>
		void set(const T& val);

		bool operator == (const Data& d) const;
		bool operator != (const Data& d) const;

		static std::map<std::string, std::function<Data()>>& factories();

	protected:
		std::string toString() const;

		std::shared_ptr<const Holder> m_data;

	private:

	friend std::ostream& operator << (std::ostream& out, const Data& bd);
};

std::ostream& operator << (std::ostream& out, const Data& bd);

}
