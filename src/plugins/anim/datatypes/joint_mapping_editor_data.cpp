#include "joint_mapping_editor_data.h"

namespace anim {

void JointMappingEditorData::setSourceSkeleton(const Skeleton& s) {
	m_sourceSkeleton = s;
}

const Skeleton& JointMappingEditorData::sourceSkeleton() const {
	return m_sourceSkeleton;
}

void JointMappingEditorData::setTargetSkeleton(const Skeleton& s) {
	m_targetSkeleton = s;
}

const Skeleton& JointMappingEditorData::targetSkeleton() const {
	return m_targetSkeleton;
}

void JointMappingEditorData::add(int fromJoint, int toJoint) {
	m_mapping.push_back(std::make_pair(fromJoint, toJoint));
}

void JointMappingEditorData::erase(iterator i) {
	m_mapping.erase(i);
}

void JointMappingEditorData::clear() {
	m_mapping.clear();
}

JointMappingEditorData::const_iterator JointMappingEditorData::begin() const {
	return m_mapping.begin();
}

JointMappingEditorData::const_iterator JointMappingEditorData::end() const {
	return m_mapping.end();
}

JointMappingEditorData::const_iterator JointMappingEditorData::findSource(int index) const {
	return std::find_if(m_mapping.begin(), m_mapping.end(),
	                    [index](const std::pair<int, int>& p) { return p.first == index; });
}

JointMappingEditorData::const_iterator JointMappingEditorData::findTarget(int index) const {
	return std::find_if(m_mapping.begin(), m_mapping.end(),
	                    [index](const std::pair<int, int>& p) { return p.second == index; });
}

JointMappingEditorData::iterator JointMappingEditorData::begin() {
	return m_mapping.begin();
}

JointMappingEditorData::iterator JointMappingEditorData::end() {
	return m_mapping.end();
}

std::pair<int, int>& JointMappingEditorData::operator[](std::size_t index) {
	return m_mapping[index];
}

const std::pair<int, int>& JointMappingEditorData::operator[](std::size_t index) const {
	return m_mapping[index];
}

bool JointMappingEditorData::empty() const {
	return m_mapping.empty();
}

std::size_t JointMappingEditorData::size() const {
	return m_mapping.size();
}

bool JointMappingEditorData::operator==(const JointMappingEditorData& d) const {
	return m_targetSkeleton == d.m_targetSkeleton && m_sourceSkeleton == d.m_sourceSkeleton && m_mapping == d.m_mapping;
}

bool JointMappingEditorData::operator!=(const JointMappingEditorData& d) const {
	return m_targetSkeleton != d.m_targetSkeleton || m_sourceSkeleton != d.m_sourceSkeleton || m_mapping != d.m_mapping;
}

std::ostream& operator<<(std::ostream& out, const JointMappingEditorData& d) {
	out << "(joint mapping editor data)";
	return out;
}

}  // namespace anim

namespace {

void toJson(::possumwood::io::json& json, const anim::JointMappingEditorData& value) {
	for(auto& i : value) {
		::possumwood::io::json jval;
		jval[0] = i.first;
		jval[1] = i.second;

		json.push_back(jval);
	}
}

void fromJson(const ::possumwood::io::json& json, anim::JointMappingEditorData& value) {
	value.clear();

	for(auto& i : json)
		value.add(i[0].get<int>(), i[1].get<int>());
}

}  // namespace

namespace possumwood {

IO<anim::JointMappingEditorData> Traits<anim::JointMappingEditorData>::io(&toJson, &fromJson);

}
