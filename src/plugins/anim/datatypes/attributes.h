#pragma once

#include <map>
#include <memory>

#include "attribute.h"

namespace anim {

class Attributes {
  public:
	Attributes();

	Attributes(const Attributes& a);
	Attributes(Attributes&& a);

	Attributes& operator=(const Attributes& a);
	Attributes& operator=(Attributes&& a);

	const Attribute& operator[](const std::string& key) const;
	Attribute& operator[](const std::string& key);

	typedef std::map<std::string, Attribute>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	typedef std::map<std::string, Attribute>::iterator iterator;
	iterator begin();
	iterator end();

  protected:
  private:
	std::map<std::string, Attribute> m_attributes;
};

std::ostream& operator<<(std::ostream& out, const Attributes& attrs);

}  // namespace anim
