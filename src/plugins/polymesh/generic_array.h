#pragma once

#include <typeinfo>
#include <typeindex>
#include <memory>
#include <vector>

namespace possumwood {
namespace polymesh {

struct ArrayBase {
	virtual ~ArrayBase() = 0;

	virtual const std::type_index type() const = 0;
	virtual std::unique_ptr<ArrayBase> clone() const = 0;

	virtual bool empty() const = 0;
	virtual std::size_t size() const = 0;

	virtual void clear() = 0;
	virtual void resize(std::size_t size) = 0;
	virtual void add(/*default*/) = 0;
	virtual bool isEqual(const ArrayBase& b) const = 0;
};

template<typename T>
class Array : public ArrayBase {
	public:
		Array(const T& defaultValue);
		~Array();

		virtual const std::type_index type() const override;

		virtual std::unique_ptr<ArrayBase> clone() const;

		const T& operator[](std::size_t index) const;
		T& operator[](std::size_t index);

		virtual bool empty() const override;
		virtual std::size_t size() const override;

		virtual void add(/*default*/) override;
		void add(const T& value);
		virtual void clear() override;
		virtual void resize(std::size_t size) override;
		virtual bool isEqual(const ArrayBase& b) const override;

	private:
		std::vector<T> m_data;
		T m_defaultValue;
};

}
}
