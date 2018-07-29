#pragma once

#include <string>
#include <memory>
#include <typeinfo>
#include <functional>

#include <boost/noncopyable.hpp>

#include "data.h"

namespace dependency_graph {

class Metadata;
class Node;
class Datablock;

/// Common attribute base class, with type erasure for its concrete Data instantiation.
/// The instance itself is copyable, even only as a base class - derived classes
/// TypedAttr, InAttr and OutAttr don't hold any data, and are intended mainly
/// as an API layer.
class Attr {
	public:
		enum Category { kInput, kOutput };

		virtual ~Attr();

		const std::string& name() const;
		const Category& category() const;
		const unsigned& offset() const;
		const std::type_info& type() const;

		bool isValid() const;

		bool operator == (const Attr& a) const;
		bool operator != (const Attr& a) const;

		/// todo: make private - no need for it in public interface
		std::unique_ptr<BaseData> createData() const;

	protected:
		Attr(const std::string& name, Category cat, const BaseData& data);

		void setOffset(unsigned o);

	private:
		struct AttrData;
		std::shared_ptr<const AttrData> m_data;

		friend class Datablock;
		friend class Node;
		friend class Metadata;
};

template<typename T>
class TypedAttr : public Attr {
	protected:
		TypedAttr(const std::string& name, Category cat, const T& defaultValue);
};

template<>
class TypedAttr<void> : public Attr {
	public:
		TypedAttr(const std::string& name, Category cat);
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttr final : public TypedAttr<T> {
	public:
		InAttr();

	protected:
		InAttr(const std::string& name, const T& defaultValue);

		friend class Metadata;
};

template<>
class InAttr<void> final : public TypedAttr<void> {
	public:
		InAttr();

	protected:
		InAttr(const std::string& name);

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttr final : public TypedAttr<T> {
	public:
		OutAttr();

	protected:
		OutAttr(const std::string& name, const T& defaultValue);

		friend class Metadata;
};

template<>
class OutAttr<void> final : public TypedAttr<void> {
	public:
		OutAttr();

	protected:
		OutAttr(const std::string& name);

		friend class Metadata;
};

std::ostream& operator << (std::ostream& out, const Attr& attr);

}
