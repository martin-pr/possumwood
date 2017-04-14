#pragma once

#include <vector>
#include <memory>

namespace dependency_graph {

class Metadata;
class Node;

template<typename T>
class InAttr;

template<typename T>
class OutAttr;

/// A data storage class used by Node implementation.
/// Each data value is strongly typed, and stored as base class pointer.
class Datablock {
	public:
		struct BaseData {
			virtual ~BaseData() {};

			virtual void assign(const BaseData& src) = 0;
			virtual bool isEqual(const BaseData& src) const = 0;
		};

		template<typename T>
		struct Data : public BaseData {
			Data(const T& v = T()) : value(v) {};
			virtual ~Data() {};
			virtual void assign(const BaseData& src) override;
			virtual bool isEqual(const BaseData& src) const;

			T value;
		};

		Datablock(const Metadata& meta);

		template<typename T>
		const T& get(const InAttr<T>& attr) const;

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

	protected:
		template<typename T>
		const T& get(size_t index) const;

		template<typename T>
		void set(size_t index, const T& value);

		const BaseData& data(size_t index) const;
		BaseData& data(size_t index);

	private:
		std::vector<std::unique_ptr<BaseData>> m_data;

		friend class Node;
};

}
