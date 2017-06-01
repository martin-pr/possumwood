#pragma once

#include <string>
#include <memory>
#include <typeinfo>

#include "datablock.h"

namespace dependency_graph {

class Metadata;
class Node;

/// Common attribute base class
class Attr {
	public:
		enum Category { kInput, kOutput };

		virtual ~Attr() = 0;

		const std::string& name() const;
		const Category& category() const;
		const unsigned& offset() const;
		virtual const std::type_info& type() const = 0;

		bool isValid() const;

		bool operator == (const Attr& a) const;
		bool operator != (const Attr& a) const;

	protected:
		Attr(const std::string& name, unsigned offset, Category cat);

		virtual std::unique_ptr<BaseData> createData() const = 0;

	private:
		std::string m_name;
		unsigned m_offset;
		Category m_category;

		friend class Datablock;
		friend class Node;
};

template<typename T>
class TypedAttr : public Attr {
	public:
		virtual const std::type_info& type() const override;

	protected:
		TypedAttr(const std::string& name, unsigned offset, Category cat, const T& defaultValue);

		virtual std::unique_ptr<BaseData> createData() const override;

	private:
		T m_defaultValue;
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttr : public TypedAttr<T> {
	public:
		InAttr();

	protected:
		InAttr(const std::string& name, unsigned offset, const T& defaultValue);

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttr : public TypedAttr<T> {
	public:
		OutAttr();

	protected:
		OutAttr(const std::string& name, unsigned offset, const T& defaultValue);

		friend class Metadata;
};

}
