#pragma once

#include <actions/traits.h>

#include <array>
#include <memory>

#include "polygons.h"
#include "skinned_vertices.h"

namespace anim {

/// A slightly sloppy version of skinned mesh data representation
class SkinnedMesh {
  public:
	SkinnedMesh();

	const std::string& name() const;
	void setName(const std::string& name);

	SkinnedVertices& vertices();
	const SkinnedVertices& vertices() const;

	std::vector<Imath::V3f>& normals();
	const std::vector<Imath::V3f>& normals() const;

	Polygons& polygons();
	const Polygons& polygons() const;

  protected:
  private:
	std::string m_name;
	SkinnedVertices m_vertices;
	std::vector<Imath::V3f> m_normals;
	Polygons m_polygons;
};
}  // namespace anim

namespace possumwood {

template <>
struct Traits<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{0, 0.5, 0.5}};
	}
};

}  // namespace possumwood
