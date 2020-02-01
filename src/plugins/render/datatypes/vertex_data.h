#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>

#include <actions/traits.h>
#include <possumwood_sdk/drawable.h>

#include "vbo.h"
#include "buffer.h"

namespace possumwood {

/// holds data about a set of vertex buffer objects
class VertexData {
  public:
	VertexData(GLenum drawElementType = GL_TRIANGLE_STRIP);

	VertexData(const VertexData&) = delete;
	VertexData& operator = (const VertexData&) = delete;

	VertexData(VertexData&&) = default;
	VertexData& operator = (VertexData&&) = default;

	/// type of update - static (updated with the DAG), per drawing (updated on each frame
	/// - usable only for camera-dependent data)
	enum UpdateType { kStatic = 1, kPerDraw = 2 };

	/// adds a generic VBO with an update functor.
	template <typename T>
	void addVBO(const std::string& name, std::size_t size,
	            const UpdateType& updateType,
	            std::function<void(Buffer<typename VBOTraits<T>::element>&, const ViewportState& viewport)> updateFn);

	/// updates and uses the program
	dependency_graph::State use(GLuint programId, const ViewportState& vs) const;

	/// returns the drawing element primitive type
	GLenum drawElementType() const;

	/// returns the size of this vertex data object (number of vertices to be drawn)
	std::size_t size() const;

	/// returns the GLSL declaration of data in this VertexData object
	std::string glslDeclaration() const;
	/// returns the number of VBOs in this object
	std::size_t vboCount() const;

	/// returns the names of all uniforms that have been registered (todo: change to iterators)
	std::set<std::string> names() const;

  private:
	struct VBOHolder {
		std::string name, glslType;
		std::unique_ptr<VBOBase> vbo;
		std::size_t size;
		UpdateType updateType;
		std::function<std::unique_ptr<BufferBase>(const ViewportState& vs)> update;
		mutable std::unique_ptr<BufferBase> buffer; // MESSY
	};

	std::vector<VBOHolder> m_vbos;
	GLenum m_drawElementType;

	friend std::ostream& operator << (std::ostream& out, const VertexData& vd);
};

std::ostream& operator << (std::ostream& out, const VertexData& vd);

template <>
struct Traits<VertexData> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 1}};
	}
};
}
