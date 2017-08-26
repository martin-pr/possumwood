#include "skeleton.h"

#include <cassert>
#include <iostream>

using std::cout;
using std::endl;

namespace anim {

Skeleton::Joint::Joint(std::size_t id, const Transform& transform, Skeleton* skel) :
	m_id(id), m_transformation(transform), m_skeleton(skel) {
}

const std::string& Skeleton::Joint::name() const {
	assert(m_skeleton != NULL);
	assert(m_id < m_skeleton->size());
	return (*m_skeleton->m_hierarchy)[m_id].name;
}

std::size_t Skeleton::Joint::index() const {
	return m_id;
}

Children<Skeleton::Joint, Skeleton> Skeleton::Joint::children() {
	assert(m_skeleton != NULL);
	auto& j = (*m_skeleton->m_hierarchy)[m_id];
	return Children<Skeleton::Joint, Skeleton>(j.children_begin, j.children_end, m_skeleton);
}

const Children<Skeleton::Joint, Skeleton> Skeleton::Joint::children() const {
	assert(m_skeleton != NULL);
	auto& j = (*m_skeleton->m_hierarchy)[m_id];
	return Children<Skeleton::Joint, Skeleton>(j.children_begin, j.children_end, m_skeleton);
}

bool Skeleton::Joint::hasParent() const {
	assert(m_skeleton != NULL);
	assert(m_id < m_skeleton->size());
	return (*m_skeleton->m_hierarchy)[m_id].parent >= 0;
}

Skeleton::Joint& Skeleton::Joint::parent() {
	assert(m_skeleton != NULL);
	assert(m_id < m_skeleton->size());
	assert(hasParent());
	return (*m_skeleton)[(*m_skeleton->m_hierarchy)[m_id].parent];
}

const Skeleton::Joint& Skeleton::Joint::parent() const {
	assert(m_skeleton != NULL);
	assert(m_id < m_skeleton->size());
	assert(hasParent());
	return (*m_skeleton)[(*m_skeleton->m_hierarchy)[m_id].parent];
}

Transform& Skeleton::Joint::tr() {
	return m_transformation;
}

const Transform& Skeleton::Joint::tr() const {
	return m_transformation;
}

Attributes& Skeleton::Joint::attributes() {
	return m_skeleton->m_hierarchy->itemAttributes(m_id);
}

const Attributes& Skeleton::Joint::attributes() const {
	return m_skeleton->m_hierarchy->itemAttributes(m_id);
}

////////

Skeleton::Skeleton() : m_hierarchy(new Hierarchy()) {
}

Skeleton::Skeleton(const Skeleton& h) : m_joints(h.m_joints), m_hierarchy(h.m_hierarchy) {
	for(auto& j : m_joints)
		j.m_skeleton = this;
}

Skeleton& Skeleton::operator = (const Skeleton& h) {
	m_hierarchy = h.m_hierarchy;
	m_joints = h.m_joints;

	for(auto& j : m_joints)
		j.m_skeleton = this;

	return *this;
}

Skeleton::Skeleton(Skeleton&& h) : m_joints(std::move(h.m_joints)), m_hierarchy(std::move(h.m_hierarchy)) {
	for(auto& j : m_joints)
		j.m_skeleton = this;
}

Skeleton& Skeleton::operator = (Skeleton&& h) {
	m_joints = std::move(h.m_joints);
	m_hierarchy = std::move(h.m_hierarchy);

	for(auto& j : m_joints)
		j.m_skeleton = this;

	return *this;
}

Skeleton::Joint& Skeleton::operator[](std::size_t index) {
	assert(index < m_joints.size());
	return m_joints[index];
}

const Skeleton::Joint& Skeleton::operator[](std::size_t index) const {
	assert(index < m_joints.size());
	return m_joints[index];
}

bool Skeleton::empty() const {
	return m_joints.empty();
}

size_t Skeleton::size() const {
	return m_joints.size();
}

std::size_t Skeleton::indexOf(const Joint& j) const {
	assert(j.m_skeleton == this);
	assert(j.children().m_joints == this);

	return (&j - &(*m_joints.begin()));
}

void Skeleton::addRoot(const std::string& name, const Transform& tr) {
	// changing the hierarchy means the result is no longer compatible with other instances sharing the same
	// hierarchy instance
	m_hierarchy = std::shared_ptr<Hierarchy>(new Hierarchy(*m_hierarchy));

	// create a single root joint, with children "behind the end"
	m_hierarchy->addRoot(name);

	// and just add a joint to the hierarchy, updating all related joints
	m_joints.insert(m_joints.begin(), Joint(0, tr, this));
	for(auto it = m_joints.begin() + 1; it != m_joints.end(); ++it)
		++it->m_id;

	assert(m_joints.size() == m_hierarchy->size());
}

std::size_t Skeleton::addChild(const Joint& j, const Transform& tr, const std::string& name) {
	assert(j.m_skeleton == this && "input joint has to be part of the current Skeleton!");

	// changing the hierarchy means the result is no longer compatible with other instances sharing the same
	// hierarchy instance
	m_hierarchy = std::shared_ptr<Hierarchy>(new Hierarchy(*m_hierarchy));

	// add a child
	std::size_t index = m_hierarchy->addChild((*m_hierarchy)[j.m_id], name);

	// and just add a joint to the hierarchy, updating all related joints
	m_joints.insert(m_joints.begin() + index, Joint(index, tr, this));
	for(auto it = m_joints.begin() + index + 1; it != m_joints.end(); ++it)
		++it->m_id;

	assert(m_joints.size() == m_hierarchy->size());

	return index;
}

Skeleton::const_iterator Skeleton::begin() const {
	return m_joints.begin();
}

Skeleton::const_iterator Skeleton::end() const {
	return m_joints.end();
}

Skeleton::iterator Skeleton::begin() {
	return m_joints.begin();
}

Skeleton::iterator Skeleton::end() {
	return m_joints.end();
}

bool Skeleton::isCompatibleWith(const Skeleton& s) const {
	// easy case - the same shared pointer means the hierarchy is shared
	if(m_hierarchy == s.m_hierarchy)
		return true;

	// more complex case - the hierarchies should be equal
	return m_hierarchy->isCompatibleWith(*s.m_hierarchy);
}

Skeleton Skeleton::operator *(const Imath::M44f& m) const {
	Skeleton result = *this;
	result *= m;
	return result;
}

Skeleton& Skeleton::operator *=(Imath::M44f m) {
	if(m_joints.size() > 0) {
		// root should be rotated, translated and scaled
		m_joints[0].tr() *= m;

		// rest should be only scaled (its translational part)
		Imath::Vec3<float> sc;
		for(unsigned a = 0; a < 3; ++a)
			sc[a] = std::sqrt(powf(m[0][a], 2) + powf(m[1][a], 2) + powf(m[2][a], 2));

		for(auto it = m_joints.begin()+1; it != m_joints.end(); ++it)
			for(unsigned a=0;a<3;++a)
				it->tr().translation[a] *= sc[a];
	}

	return *this;
}


///

namespace {
void printBone(std::ostream& out, const Skeleton& skel, unsigned index, const std::string& prepend = "") {
	out << prepend << skel[index].name() << " [" << index << "]   " << skel[index].tr() << endl;
	for(auto& chld : skel[index].children())
		printBone(out, skel, chld.index(), prepend + "  ");
}
}

std::ostream& operator << (std::ostream& out, const Skeleton& skel) {
	if(!skel.empty())
		printBone(out, skel, 0);

	return out;
}

Attributes& Skeleton::attributes() {
	return m_hierarchy->attributes();
}

const Attributes& Skeleton::attributes() const {
	return m_hierarchy->attributes();
}

}
