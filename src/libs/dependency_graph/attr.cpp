#include "attr.inl"

namespace dependency_graph {

struct Attr::AttrData {
	AttrData(const AttrData&) = delete;
	AttrData& operator=(const AttrData&) = delete;

	std::string name;
	unsigned offset;
	Category category;

	std::unique_ptr<BaseData> data;
};

Attr::Attr(const std::string& name, Category cat, const BaseData& d)
	: m_data(new AttrData{name, unsigned(-1), cat, d.clone()}) {
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
		m_data->data->clone()
	});

	m_data = std::shared_ptr<const AttrData>(newData.release());
}

const std::type_info& Attr::type() const {
	return m_data->data->typeinfo();
}

std::unique_ptr<BaseData> Attr::createData() const {
	return m_data->data->clone();
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

TypedAttr<void>::TypedAttr(const std::string& name, Category cat) :
	Attr(name, cat, Data<void>()) {
}

InAttr<void>::InAttr() : TypedAttr<void>("", Attr::kInput) {
}

InAttr<void>::InAttr(const std::string& name) : TypedAttr<void>(name, Attr::kInput) {
}

OutAttr<void>::OutAttr() : TypedAttr<void>("", Attr::kOutput) {
}

OutAttr<void>::OutAttr(const std::string& name) : TypedAttr<void>(name, Attr::kOutput) {
}

}
