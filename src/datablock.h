#pragma once

#include <vector>
#include <memory>

class Node;
class Metadata;

template<typename T>
class InAttr;

template<typename T>
class OutAttr;

class Datablock {
	public:
		template<typename T>
		const T& get(const InAttr<T>& attr) const;

		template<typename T>
		void set(const OutAttr<T>& attr, const T& value);

	protected:
	private:
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
		void set(const InAttr<T>& attr, const T& value);

		std::vector<std::unique_ptr<BaseData>> m_data;

		friend class Node;
		friend class Attr;
};
