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
}
