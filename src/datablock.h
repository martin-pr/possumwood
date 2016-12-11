#pragma once

#include <vector>
#include <memory>

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
		};

		template<typename T>
		struct Data : public BaseData {
			virtual ~Data() {};
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

	private:
		std::vector<std::unique_ptr<BaseData>> m_data;

		friend class Node;
};
