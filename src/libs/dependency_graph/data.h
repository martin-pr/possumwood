#pragma once

#include <memory>
#include <functional>
#include <type_traits>
#include <map>

#include <boost/noncopyable.hpp>

namespace dependency_graph {

class BaseData {
	public:
		virtual ~BaseData();

		virtual void assign(const BaseData& src) = 0;
		virtual bool isEqual(const BaseData& src) const = 0;

		std::string type() const;
		virtual const std::type_info& typeinfo() const = 0;

		/// creates a new Data<T> instance based on type name
		static std::unique_ptr<BaseData> create(const std::string& type);
		/// clones an existing BaseData instance
		virtual std::unique_ptr<BaseData> clone() const = 0;

	protected:
		virtual std::string toString() const = 0;

		BaseData();

		BaseData(const BaseData& bd);
		BaseData& operator = (const BaseData& bd);

		template<typename T>
		struct Factory {
			Factory();
		};

	private:
		static std::map<std::string, std::function<std::unique_ptr<BaseData>()>>& factories();

	friend std::ostream& operator << (std::ostream& out, const BaseData& bd);
};

template<typename T>
struct Data : public BaseData {
	public:
		Data(const T& v = T());
		virtual ~Data();

		virtual void assign(const BaseData& src) override;
		virtual bool isEqual(const BaseData& src) const override;

		virtual const std::type_info& typeinfo() const override;

		std::unique_ptr<BaseData> clone() const override;

		T value;

	protected:
		virtual std::string toString() const override;

	private:
		static Factory<T> m_factory;
};

template<>
struct Data<void> : public BaseData {
	public:
		Data();
		virtual ~Data();

		virtual void assign(const BaseData& src) override;
		virtual bool isEqual(const BaseData& src) const override;

		virtual const std::type_info& typeinfo() const override;

		std::unique_ptr<BaseData> clone() const override;

	private:
		virtual std::string toString() const override;

		static Factory<void> m_factory;
};

std::ostream& operator << (std::ostream& out, const BaseData& bd);

}
