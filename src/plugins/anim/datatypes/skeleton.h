#pragma once

#include <vector>
#include <string>
#include <memory>

#include <boost/noncopyable.hpp>

#include <actions/traits.h>

#include "hierarchy.h"
#include "transform.h"

namespace anim {

class Skeleton {
	public:
		class Joint;

		typedef std::vector<Joint>::const_iterator const_iterator;
		typedef std::vector<Joint>::iterator iterator;

		/// a single joint and its children.
		/// The joint instance behaves like an iterator - it is just a weak
		/// reference to the Hierarchy structure and cannot exist on its own.
		class Joint {
			public:
				const std::string& name() const;
				std::size_t index() const;

				Children<Joint, Skeleton> children();
				const Children<Joint, Skeleton> children() const;

				bool hasParent() const;
				Joint& parent();
				const Joint& parent() const;

				// local space transformation
				Transform& tr();
				const Transform& tr() const;

				// compute world space transformation
				Transform world() const;

				/// returns joint attributes (shared between all hierarchy instances)
				Attributes& attributes();
				/// returns joint attributes (shared between all hierarchy instances)
				const Attributes& attributes() const;

				bool operator == (const Joint& j) const;
				bool operator != (const Joint& j) const;

			private:
				Joint(std::size_t id, const Transform& transform, Skeleton* skel);

				std::size_t m_id;
				Transform m_transformation;
				Skeleton* m_skeleton;

			friend class Skeleton;
		};

		Skeleton();
		Skeleton(const Skeleton& h);
		Skeleton& operator = (const Skeleton& h);
		Skeleton(Skeleton&& h);
		Skeleton& operator = (Skeleton&& h);

		Joint& operator[](std::size_t index);
		const Joint& operator[](std::size_t index) const;
		std::size_t indexOf(const Joint& j) const;

		bool empty() const;
		size_t size() const;

		void addRoot(const std::string& name, const Transform& tr);
		std::size_t addChild(const Joint& j, const Transform& tr, const std::string& name);

		const_iterator begin() const;
		const_iterator end() const;

		iterator begin();
		iterator end();

		/// returns true if the poses between these two skeletons can be directly assigned (if they share the same hierarchy instance)
		bool isCompatibleWith(const Skeleton& s) const;

		/// returns hierarchy attributes (shared between skeleton instances)
		Attributes& attributes();
		/// returns hierarchy attributes (shared between skeleton instances)
		const Attributes& attributes() const;

		/// transforms this skeleton using a transformation matrix
		Skeleton operator *(const Imath::M44f& m) const;
		/// transforms this skeleton using a transformation matrix
		Skeleton& operator *=(Imath::M44f m);

		bool operator == (const Skeleton& skel) const;
		bool operator != (const Skeleton& skel) const;

	protected:
		/// makes two skeletons share the same hierarchy structure (assumes isCompatibleWith() == true).
		/// Used by the Animation class.
		void makeConsistentWith(Skeleton& s);

	private:
		// exists only so I can return references to joints, not instances
		std::vector<Joint> m_joints;
		// stores the hierachy of joints, shared between all "compatible" skeleton instances
		// (instances whose poses can be directly assigned).
		std::shared_ptr<Hierarchy> m_hierarchy;

	friend class Animation;
};

std::ostream& operator << (std::ostream& out, const Skeleton& skel);

}

namespace possumwood {

template<>
struct Traits<anim::Skeleton> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0.5, 0}};
	}
};

}
