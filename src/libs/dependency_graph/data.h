#pragma once

#include <memory>
#include <functional>
#include <type_traits>
#include <map>

#include <boost/noncopyable.hpp>

namespace dependency_graph {

template<typename T>
class TypedData;

class Data {
	public:
		virtual ~Data();

		virtual void assign(const Data& src) = 0;
		virtual bool isEqual(const Data& src) const = 0;

		std::string type() const;
		virtual const std::type_info& typeinfo() const = 0;

		/// creates a new Data<T> instance based on type name
		static std::unique_ptr<Data> create(const std::string& type);
		/// clones an existing Data instance
		virtual std::unique_ptr<Data> clone() const = 0;

		template<typename T>
		const T& get() const {
			if(std::string(typeinfo().name()) == typeid(void).name())
				throw std::runtime_error("Attempting to use a void value!");

			const TypedData<T>* tmp = dynamic_cast<const TypedData<T>*>(this);
			if(!tmp)
				throw std::runtime_error(std::string("Invalid type requested - requested ") + typeid(T).name() + ", found " + typeinfo().name());

			return tmp->get();
		}

		template<typename T>
		void set(const T& val) {
			TypedData<T>* tmp = dynamic_cast<TypedData<T>*>(this);
			assert(tmp);
			tmp->set(val);
		}

	protected:
		virtual std::string toString() const = 0;

		Data();

		Data(const Data& bd);
		Data& operator = (const Data& bd);

		template<typename T>
		struct Factory {
			Factory();
		};

	private:
		static std::map<std::string, std::function<std::unique_ptr<Data>()>>& factories();

	friend std::ostream& operator << (std::ostream& out, const Data& bd);
};

template<typename T>
struct TypedData : public Data {
	public:
		TypedData(const T& v = T());
		virtual ~TypedData();

		virtual void assign(const Data& src) override;
		virtual bool isEqual(const Data& src) const override;

		virtual const std::type_info& typeinfo() const override;

		std::unique_ptr<Data> clone() const override;

	protected:
		virtual std::string toString() const override;

	private:
		const T& get() const;
		void set(const T& val);

		T m_value;

		static Factory<T> m_factory;

	friend class Data;
};

template<>
struct TypedData<void> : public Data {
	public:
		TypedData();
		virtual ~TypedData();

		virtual void assign(const Data& src) override;
		virtual bool isEqual(const Data& src) const override;

		virtual const std::type_info& typeinfo() const override;

		std::unique_ptr<Data> clone() const override;

	private:
		virtual std::string toString() const override;

		static Factory<void> m_factory;
};

std::ostream& operator << (std::ostream& out, const Data& bd);

}
