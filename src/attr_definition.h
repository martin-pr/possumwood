#pragma once

#include <string>

class Metadata;

/// Common attribute base class
class AttrDefinition {
	public:
		enum Category { kInput, kOutput };

		virtual ~AttrDefinition = 0;

		const std::string& name() const;
		const Category& category() const;
		const unsigned& index() const;

	protected:
		AttrDefinition(const std::string& name, unsigned index);
};

/// Input attribute type (constructed by Metadata class)
template<typename T>
class InAttrDefinition : public AttrDefinition {
	public:
		InAttrDefinition();

		bool isValid();

	protected:
		InAttrDefinition(const std::string& name, unsigned index);

		friend class Metadata;
};

/// Output attribute type (constructed by Metadata class)
template<typename T>
class OutAttrDefinition : public AttrDefinition {
	public:
		OutAttrDefinition();

		bool isValid();

	protected:
		OutAttrDefinition(const std::string& name, unsigned index);

		friend class Metadata;
};
