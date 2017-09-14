#include "skin_mapping_editor_data.h"

namespace anim {

void SkinMappingEditorData::setSkeleton(const Skeleton& s) {
	m_skeleton = s;
}

const Skeleton& SkinMappingEditorData::skeleton() const {
	return m_skeleton;
}

void SkinMappingEditorData::add(int fromJoint, int toJoint) {
	m_mapping.push_back(std::make_pair(fromJoint, toJoint));
}

void SkinMappingEditorData::erase(iterator i) {
	m_mapping.erase(i);
}

void SkinMappingEditorData::clear() {
	m_mapping.clear();
}

SkinMappingEditorData::const_iterator SkinMappingEditorData::begin() const {
	return m_mapping.begin();
}

SkinMappingEditorData::const_iterator SkinMappingEditorData::end() const {
	return m_mapping.end();
}

SkinMappingEditorData::iterator SkinMappingEditorData::begin() {
	return m_mapping.begin();
}

SkinMappingEditorData::iterator SkinMappingEditorData::end() {
	return m_mapping.end();
}

std::pair<int, int>& SkinMappingEditorData::operator[](std::size_t index) {
	return m_mapping[index];
}

const std::pair<int, int>& SkinMappingEditorData::operator[](std::size_t index) const {
	return m_mapping[index];
}


bool SkinMappingEditorData::empty() const {
	return m_mapping.empty();
}

std::size_t SkinMappingEditorData::size() const {
	return m_mapping.size();
}

bool SkinMappingEditorData::operator==(const SkinMappingEditorData& d) const {
	return m_skeleton == d.m_skeleton && m_mapping == d.m_mapping;
}
bool SkinMappingEditorData::operator!=(const SkinMappingEditorData& d) const {
	return m_skeleton != d.m_skeleton || m_mapping != d.m_mapping;
}

}

namespace {

void toJson(::dependency_graph::io::json& json, const anim::SkinMappingEditorData& value) {
	for(auto& i : value) {
		::dependency_graph::io::json jval;
		jval[0] = i.first;
		jval[1] = i.second;

		json.push_back(jval);
	}
}

void fromJson(const ::dependency_graph::io::json& json, anim::SkinMappingEditorData& value) {
	value.clear();

	for(auto& i : json)
		value.add(i[0].get<int>(), i[1].get<int>());
}

}

namespace possumwood {

IO<anim::SkinMappingEditorData> Traits<anim::SkinMappingEditorData>::io(&toJson, &fromJson);

}
