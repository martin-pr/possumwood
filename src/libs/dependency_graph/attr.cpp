#include "attr.inl"

#include "datablock.h"

namespace dependency_graph {

struct Attr::AttrData {
	AttrData(const AttrData&) = delete;
	AttrData& operator=(const AttrData&) = delete;

	std::string name;
	unsigned offset;
	Category category;
	unsigned flags;

	Data data;
};

Attr::Attr(const std::string& name, Category cat, const Data& d, unsigned flags)
	: m_data(new AttrData{name, unsigned(-1), cat, flags, d}) {
}

Attr::~Attr() {
}

const std::string& Attr::name() const {
	return m_data->name;
}

const Attr::Category& Attr::category() const {
	return m_data->category;
}

const unsigned& Attr::offset() const {
	return m_data->offset;
}

void Attr::setOffset(unsigned o) {
	std::unique_ptr<AttrData> newData(new AttrData{
		m_data->name,
		o,
		m_data->category,
		m_data->flags,
		m_data->data
	});

	m_data = std::shared_ptr<const AttrData>(newData.release());
}

const std::type_info& Attr::type() const {
	return m_data->data.typeinfo();
}

unsigned Attr::flags() const {
	return m_data->flags;
}

Data Attr::createData() const {
	return m_data->data;
}

bool Attr::isValid() const {
	return m_data->offset != unsigned(-1);
}

bool Attr::operator==(const Attr& a) const {
	return name() == a.name() && category() == a.category() && offset() == a.offset() &&
	       type() == a.type();
}

bool Attr::operator!=(const Attr& a) const {
	return name() != a.name() || category() != a.category() || offset() != a.offset() ||
	       type() != a.type();
}

std::ostream& operator << (std::ostream& out, const Attr& attr) {
	out << attr.name() << " (";
	if(attr.category() == Attr::kInput)
		out << "input, ";
	else
		out << "output, ";
	out << attr.type().name() << ")";

	return out;
}

/////////

TypedAttr<void>::TypedAttr(const std::string& name, Category cat, unsigned flags) :
	Attr(name, cat, Data(), flags) {
}

InAttr<void>::InAttr() : TypedAttr<void>("", Attr::kInput, 0) {
}

InAttr<void>::InAttr(const std::string& name, unsigned flags) : TypedAttr<void>(name, Attr::kInput, flags) {
}

OutAttr<void>::OutAttr() : TypedAttr<void>("", Attr::kOutput, 0) {
}

OutAttr<void>::OutAttr(const std::string& name, unsigned flags) : TypedAttr<void>(name, Attr::kOutput, flags) {
}

}
