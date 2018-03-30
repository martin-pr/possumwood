#pragma once

#include <memory>
#include <functional>
#include <type_traits>

#include "io/json.h"

namespace dependency_graph {

class BaseData {
	public:
		virtual ~BaseData() {};

		virtual void assign(const BaseData& src) = 0;
		virtual bool isEqual(const BaseData& src) const = 0;

		virtual std::string type() const = 0;

		/// creates a new Data<T> instance based on type name
		static std::unique_ptr<BaseData> create(const std::string& type);
		/// clones an existing BaseData instance
		virtual std::unique_ptr<BaseData> clone() const = 0;

	protected:
		template<typename T>
		struct Factory {
			Factory();
		};

	private:
		static std::map<std::string, std::function<std::unique_ptr<BaseData>()>>& factories();
};

template<typename T>
struct Data : public BaseData {
	public:
		Data(const T& v = T());
		virtual ~Data();

		virtual void assign(const BaseData& src) override;
		virtual bool isEqual(const BaseData& src) const;

		virtual std::string type() const override;

		std::unique_ptr<BaseData> clone() const override;

		T value;

	private:
		static Factory<T> m_factory;
};

template<>
struct Data<void> {
	// invalid
};

}
