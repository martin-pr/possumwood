#include "frame_editor_data.h"

namespace anim {

void FrameEditorData::setSkeleton(const Skeleton& s) {
	m_skeleton = s;
}

const Skeleton& FrameEditorData::skeleton() const {
	return m_skeleton;
}

void FrameEditorData::setTransform(std::size_t joint, const Transform& tr) {
	m_transforms[joint] = tr;
}

void FrameEditorData::resetTransform(std::size_t joint) {
	auto it = m_transforms.find(joint);
	if(it != m_transforms.end())
		m_transforms.erase(it);
}

Transform FrameEditorData::transform(std::size_t joint) const {
	auto it = m_transforms.find(joint);
	if(it != m_transforms.end())
		return it->second;
	return Transform();
}

void FrameEditorData::clear() {
	m_transforms.clear();
}

FrameEditorData::const_iterator FrameEditorData::begin() const {
	return m_transforms.begin();
}

FrameEditorData::const_iterator FrameEditorData::end() const {
	return m_transforms.end();
}

std::size_t FrameEditorData::size() const {
	return m_transforms.size();
}

bool FrameEditorData::operator==(const FrameEditorData& d) const {
	return m_skeleton == d.m_skeleton && m_transforms == d.m_transforms;
}

bool FrameEditorData::operator!=(const FrameEditorData& d) const {
	return m_skeleton != d.m_skeleton || m_transforms != d.m_transforms;
}

std::ostream& operator << (std::ostream& out, const FrameEditorData& d) {
	std::cout << "(frame editor data)";
	return out;
}

}

namespace {

void toJson(::possumwood::io::json& json, const anim::FrameEditorData& value) {
	for(auto& i : value) {
		::possumwood::io::json jval;
		jval[0] = i.first;
		for(unsigned a=0;a<4;++a)
			jval[1][a] = i.second.rotation[a];

		json.push_back(jval);
	}
}

void fromJson(const ::possumwood::io::json& json, anim::FrameEditorData& value) {
	value.clear();

	for(auto& i : json) {
		Imath::Quatf q;
		for(unsigned a=0;a<4;++a)
			q[a] = i[1][a];
		value.setTransform(i[0].get<std::size_t>(), anim::Transform(q));
	}
}

}

namespace possumwood {

IO<anim::FrameEditorData> Traits<anim::FrameEditorData>::io(&toJson, &fromJson);

}
