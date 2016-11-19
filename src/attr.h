#pragma once

#include <string>

class Metadata;

/// Common attribute base class
class Attr {
	public:
		enum Category { kInput, kOutput };

		virtual ~Attr() = 0;

		const std::string& name() const;
		const Category& category() const;
		const unsigned& offset() const;

	protected:
		Attr(const std::string& name, unsigned offset);
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttr : public Attr {
	public:
		InAttr();

		bool isValid();

	protected:
		InAttr(const std::string& name, unsigned offset);

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttr : public Attr {
	public:
		OutAttr();

		bool isValid();

	protected:
		OutAttr(const std::string& name, unsigned offset);

		friend class Metadata;
};
