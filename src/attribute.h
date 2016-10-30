#pragma once

#include <string>

class Metadata;

/// Common attribute base class
class Attribute {
	public:
		enum Category { kInput, kOutput };

		virtual ~Attribute = 0;

		const std::string& name() const;
		const Category& category() const;
		const unsigned& index() const;

	protected:
		Attribute(const std::string& name, unsigned index);
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttribute : public Attribute {
	public:
		InAttribute();

		bool isValid();

	protected:
		InAttribute(const std::string& name, unsigned index);

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttribute : public Attribute {
	public:
		OutAttribute();

		bool isValid();

	protected:
		OutAttribute(const std::string& name, unsigned index);

		friend class Metadata;
};
