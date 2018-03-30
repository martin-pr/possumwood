#include "attr.inl"

namespace dependency_graph {

struct Attr::AttrData {
	AttrData(const AttrData&) = delete;
	AttrData& operator=(const AttrData&) = delete;

	std::string name;
	unsigned offset;
	Category category;
	const std::type_info& type;

	std::function<std::unique_ptr<BaseData>()> dataFactory;
};

Attr::Attr(const std::string& name, unsigned offset, Category cat, const std::type_info& type,
           std::function<std::unique_ptr<BaseData>()> dataFactory)
	: m_data(new AttrData{name, offset, cat, type, dataFactory}) {
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

const std::type_info& Attr::type() const {
	return m_data->type;
}

std::unique_ptr<BaseData> Attr::createData() const {
	return m_data->dataFactory();
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

TypedAttr<void>::TypedAttr(const std::string& name, unsigned offset, Category cat) :
	Attr(name, offset, cat, typeid(void), []() {
		return std::unique_ptr<BaseData>();
	}) {
}

InAttr<void>::InAttr() : TypedAttr<void>("", unsigned(-1), Attr::kInput) {
}

InAttr<void>::InAttr(const std::string& name, unsigned offset) : TypedAttr<void>(name, offset, Attr::kInput) {
}

OutAttr<void>::OutAttr() : TypedAttr<void>("", unsigned(-1), Attr::kOutput) {
}

OutAttr<void>::OutAttr(const std::string& name, unsigned offset) : TypedAttr<void>(name, offset, Attr::kOutput) {
}

}
