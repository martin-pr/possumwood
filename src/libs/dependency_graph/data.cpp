#include "data.inl"

namespace dependency_graph {

std::map<std::string, std::function<std::unique_ptr<BaseData>()>>& BaseData::factories() {
	static std::map<std::string, std::function<std::unique_ptr<BaseData>()>> s_factories;
	return s_factories;
}

std::unique_ptr<BaseData> BaseData::create(const std::string& type) {
	auto it = factories().find(type);
	assert(it != factories().end());

	return it->second();
}

}
