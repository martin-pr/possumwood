#pragma once

#include "vertex_data.h"

#include "vbo.inl"

namespace possumwood {

namespace {
	template<typename T>
	struct VertexDataTypeCommon {};

	template<>
	struct VertexDataTypeCommon<float> {
		static constexpr GLenum type() { return GL_FLOAT; }
		protected:
			static constexpr const char* prefix() { return ""; }
	};

	template<>
	struct VertexDataTypeCommon<double> {
		static constexpr GLenum type() { return GL_DOUBLE; }
		protected:
			static constexpr const char* prefix() { return "" /*"d"*/; }
	};


	template<typename T, std::size_t WIDTH>
	struct VertexDataType {};

	template<>
	struct VertexDataType<float, 1> : public VertexDataTypeCommon<float> {
		static std::string glslType() { return "float"; }
	};

	template<>
	struct VertexDataType<double, 1> : public VertexDataTypeCommon<double> {
		static std::string glslType() { return "double"; }
	};

	template<typename T>
	struct VertexDataType<T, 2> : public VertexDataTypeCommon<T> {
		static std::string glslType() { return VertexDataTypeCommon<T>::prefix() + std::string("vec2"); }
	};

	template<typename T>
	struct VertexDataType<T, 3> : public VertexDataTypeCommon<T> {
		static std::string glslType() { return VertexDataTypeCommon<T>::prefix() + std::string("vec3"); }
	};

	template<typename T>
	struct VertexDataType<T, 4> : public VertexDataTypeCommon<T> {
		static std::string glslType() { return VertexDataTypeCommon<T>::prefix() + std::string("vec4"); }
	};
}

template <typename T, std::size_t WIDTH>
void VertexData::addVBO(const std::string& name, std::size_t size, std::size_t arraySize, const UpdateType& updateType,
                        std::function<void(Buffer<T, WIDTH>&)> updateFn) {
	assert(size > 0);
	assert(m_vbos.empty() || m_vbos[0].size == size);

	std::unique_ptr<VBO<T, WIDTH>> vbo(new VBO<T, WIDTH>(1, size));

	VBOHolder holder;
	holder.name = name;
	holder.glslType = std::string("in ") + VertexDataType<T, WIDTH>::glslType() + " " + name + ";";
	holder.size = size;
	holder.arraySize = size;
	holder.updateType = updateType;

	VBO<T, WIDTH>* vboPtr = vbo.get();

	holder.update = [size, vboPtr, updateFn]() {
		Buffer<T, WIDTH> buffer(1, size);

		updateFn(buffer);
		vboPtr->init(buffer);
	};

	holder.vbo = std::move(vbo);

	// update all buffers that are not per-draw
	if(holder.updateType != kPerDraw)
		holder.update();

	m_vbos.push_back(std::move(holder));
}
}
