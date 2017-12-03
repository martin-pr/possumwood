#pragma once

#include <functional>
#include <memory>

#include <boost/noncopyable.hpp>

#include <possumwood_sdk/traits.h>

#include "vbo.h"

namespace possumwood {

/// holds data about a set of vertex buffer objects
class VertexData : public boost::noncopyable {
  public:
	VertexData(GLenum drawElementType);

	/// type of update - static (updated with the DAG), per drawing (updated on each frame
	/// - usable only for camera-dependent data)
	enum UpdateType { kStatic = 1, kPerDraw = 2 };

	/// adds a generic VBO with an update functor.
	/// T can be either float or int, and WIDTH should be between 1 and 4
	template <typename T>
	void addVBO(const std::string& name, std::size_t size, std::size_t arraySize, std::size_t width,
	            const UpdateType& updateType,
	            std::function<void(Buffer<T>&)> updateFn);

	/// updates and uses the program
	void use(GLuint programId) const;

	/// returns the drawing element primitive type
	GLenum drawElementType() const;

	/// returns the size of this vertex data object (number of vertices to be drawn)
	std::size_t size() const;
	/// returns the number of array elements per vertex (1 = not using arrays)
	std::size_t arraySize() const;

	/// returns the GLSL declaration of data in this VertexData object
	std::string glslDeclaration() const;
	/// returns the number of VBOs in this object
	std::size_t vboCount() const;

  private:
	struct VBOHolder {
		std::string name, glslType;
		std::unique_ptr<VBOBase> vbo;
		std::size_t size, arraySize, width;
		UpdateType updateType;
		std::function<void()> update;
	};

	std::vector<VBOHolder> m_vbos;
	GLenum m_drawElementType;
};

template <>
struct Traits<std::shared_ptr<const VertexData>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.5, 1}};
	}
};
}
