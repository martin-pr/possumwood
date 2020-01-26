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

class Data {
	public:
		// creates empty data holder(null data with void type)
		Data();

		template<typename T>
		Data(const T& value) : m_data(new TypedHolder<T>(value)) {
		}

		void assign(const Data& src) {
			assert(std::string(src.typeinfo().name()) == std::string(typeinfo().name()));
			m_data = src.m_data;
		}

		bool isEqual(const Data& src) const {
			assert(m_data != nullptr && src.m_data != nullptr);

			return m_data->isEqual(*src.m_data);
		}

		std::string type() const;
		const std::type_info& typeinfo() const;

		/// creates a new Data<T> instance based on type name
		static std::unique_ptr<Data> create(const std::string& type);
		/// clones an existing Data instance
		std::unique_ptr<Data> clone() const;

		template<typename T>
		const T& get() const {
			if(std::string(typeinfo().name()) == typeid(void).name() || m_data == nullptr)
				throw std::runtime_error("Attempting to use a void value!");

			const TypedHolder<T>* tmp = dynamic_cast<const TypedHolder<T>*>(m_data.get());
			if(!tmp)
				throw std::runtime_error(std::string("Invalid type requested - requested ") + typeid(T).name() + ", found " + typeinfo().name());

			return tmp->data;
		}

		template<typename T>
		void set(const T& val) {
			assert(std::string(typeid(T).name()) == std::string(typeinfo().name()));

			m_data = std::shared_ptr<const Holder>(new TypedHolder<T>(val));
		}

	protected:
		std::string toString() const {
			if(m_data == nullptr)
				return "(null)";
			return m_data->toString();
		}

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
				TypedHolder(const T& d) : data(d) {
				}
				virtual ~TypedHolder() {
					// just do something with the factory, to make sure it is instantiated
					std::stringstream ss;
					ss << &m_factory;

				}

				virtual const std::type_info& typeinfo() const override {
					return typeid(T);
				}

				virtual bool isEqual(const Holder& src) const override {
					const TypedHolder<T>* h2 = dynamic_cast<const TypedHolder<T>*>(&src);
					return h2 != nullptr && DataTraits<T>::isEqual(data, h2->data);
				}

				virtual std::string toString() const override {
					std::stringstream ss;
					ss << data;
					return ss.str();
				}

				T data;
				static Factory<T> m_factory;
		};

		std::shared_ptr<const Holder> m_data;

	private:
		static std::map<std::string, std::function<std::unique_ptr<Data>()>>& factories();

	friend std::ostream& operator << (std::ostream& out, const Data& bd);
};

// template<typename T>
// class TypedData : public Data {
// 	public:
// 		TypedData(const T& v = T());
// 		virtual ~TypedData();

// 		virtual void assign(const Data& src) override;
// 		virtual bool isEqual(const Data& src) const override;

// 		virtual const std::type_info& typeinfo() const override;

// 		std::unique_ptr<Data> clone() const override;

// 	protected:
// 		virtual std::string toString() const override;

// 	friend class Data;
// };

// template<>
// struct TypedData<void> : public Data {
// 	public:
// 		TypedData();
// 		virtual ~TypedData();

// 		virtual void assign(const Data& src) override;
// 		virtual bool isEqual(const Data& src) const override;

// 		virtual const std::type_info& typeinfo() const override;

// 		std::unique_ptr<Data> clone() const override;

// 	private:
// 		virtual std::string toString() const override;

// 		static Factory<void> m_factory;
// };

std::ostream& operator << (std::ostream& out, const Data& bd);

}
