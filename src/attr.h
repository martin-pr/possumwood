#pragma once

#include <string>
#include <memory>

#include "datablock.h"

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

		bool isValid() const;

	protected:
		Attr(const std::string& name, unsigned offset, Category cat);

		virtual std::unique_ptr<Datablock::BaseData> createData() const = 0;

	private:
		std::string m_name;
		unsigned m_offset;
		Category m_category;

		friend class Datablock;
		friend class Node;
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttr : public Attr {
	public:
		InAttr();

	protected:
		InAttr(const std::string& name, unsigned offset);

		virtual std::unique_ptr<Datablock::BaseData> createData() const override;

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttr : public Attr {
	public:
		OutAttr();

	protected:
		OutAttr(const std::string& name, unsigned offset);

		virtual std::unique_ptr<Datablock::BaseData> createData() const override;

		friend class Metadata;
};
