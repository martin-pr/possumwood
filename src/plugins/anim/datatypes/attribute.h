#pragma once

#include <memory>

#include <boost/noncopyable.hpp>

#include <dependency_graph/rtti.h>

#include "../lexical_cast.h"

namespace anim {

/// A generic "attribute" holder allows to store arbitrary additional
/// type in an opaque and copyable manner. Used for storing "custom"
/// non-hardwired attributes on objects (e.g., extra data provided
/// by a loader of a generic type).
class Attribute {
	public:
		Attribute();
		~Attribute();

		Attribute(const Attribute& a);
		Attribute(Attribute&& a);

		Attribute& operator = (const Attribute& a);
		Attribute& operator = (Attribute&& a);

		std::string type() const;
		std::string toString() const;

		bool empty() const;

		template<typename T>
		bool is() const;

		template<typename T>
		const T& as() const;

		template<typename T>
		T& as();

		template<typename T>
		Attribute& operator = (const T& value);

		Attribute& operator = (const char* value);

	protected:
	private:
		class Data : public boost::noncopyable {
			public:
				Data();
				virtual ~Data();

				virtual std::string type() const = 0;
				virtual std::string toString() const = 0;

				virtual std::unique_ptr<Data> clone() const = 0;

			protected:
			private:
		};

		template<typename T>
		class DataTyped : public Data {
			public:
				DataTyped(const T& val = T());
				virtual ~DataTyped();

				virtual std::string type() const override;
				virtual std::string toString() const override;

				virtual std::unique_ptr<Data> clone() const override;

				T& operator*();
				const T& operator*() const;

			protected:
			private:
				T m_value;
		};

		std::unique_ptr<Data> m_data;
};

////

template<typename T>
bool Attribute::is() const {
	return dynamic_cast<DataTyped<T>*>(m_data.get()) != NULL;
}

template<typename T>
const T& Attribute::as() const {
	return *(dynamic_cast<DataTyped<T>&>(*m_data.get()));
}

template<typename T>
T& Attribute::as() {
	if(m_data.get() == NULL)
		m_data = std::unique_ptr<Data>(new DataTyped<T>());

	return *(dynamic_cast<DataTyped<T>&>(*m_data.get()));
}

template<typename T>
Attribute& Attribute::operator = (const T& value) {
	if(m_data.get() == NULL)
		m_data = std::unique_ptr<Data>(new DataTyped<T>(value));

	else {
		DataTyped<T>& current = dynamic_cast<DataTyped<T>&>(*m_data);
		*current = value;
	}

	return *this;
}

////

template<typename T>
Attribute::DataTyped<T>::DataTyped(const T& val) : m_value(val) {
}

template<typename T>
Attribute::DataTyped<T>::~DataTyped() {
}

template<typename T>
std::string Attribute::DataTyped<T>::type() const {
	return dependency_graph::unmangledTypeId<T>();
}

template<typename T>
std::string Attribute::DataTyped<T>::toString() const {
	return lexical_cast<std::string>(m_value);
}

template<typename T>
std::unique_ptr<Attribute::Data> Attribute::DataTyped<T>::clone() const {
	return std::unique_ptr<Data>(new DataTyped<T>(**this));
}

template<typename T>
T& Attribute::DataTyped<T>::operator*() {
	return m_value;
}

template<typename T>
const T& Attribute::DataTyped<T>::operator*() const {
	return m_value;
}

}
