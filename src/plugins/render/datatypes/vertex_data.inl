#pragma once

#include "vertex_data.h"

#include "vbo.inl"

namespace possumwood {

namespace {
	template<typename T>
	struct VertexDataType {};

	template<>
	struct VertexDataType<float> {
		static std::string glslType(std::size_t width) {
			switch(width) {
				case 1:	return "float";
				case 2:	return "vec2";
				case 3:	return "vec3";
				case 4:	return "vec4";
			};

			assert(false);
			return "unknown";
		}
	};

	template<>
	struct VertexDataType<double> {
		static std::string glslType(std::size_t width) {
			switch(width) {
				case 1:	return "double";
				case 2:	return "vec2" /*"dvec2"*/;
				case 3:	return "vec3" /*"dvec3"*/;
				case 4:	return "vec4" /*"dvec4"*/;
			};

			assert(false);
			return "unknown";
		}
	};

	template<typename T>
	std::string makeVBOName(std::size_t arraySize, std::size_t width, const std::string& name) {
		std::stringstream ss;
		ss << "in " << VertexDataType<T>::glslType(width) + " " + name;

		assert(arraySize > 0);
		if(arraySize > 1)
			ss << "[" << arraySize << "]";

		ss << ";";

		return ss.str();
	}
}

template <typename T>
void VertexData::addVBO(const std::string& name, std::size_t size, std::size_t arraySize, std::size_t width, const UpdateType& updateType,
                        std::function<void(Buffer<T>&)> updateFn) {
	assert(size > 0);
	assert(m_vbos.empty() || m_vbos[0].size == size);

	std::unique_ptr<VBO<T>> vbo(new VBO<T>(size, arraySize, width));

	VBOHolder holder;
	holder.name = name;
	holder.glslType = makeVBOName<T>(arraySize, width, name);
	holder.size = size;
	holder.arraySize = arraySize;
	holder.width = width;
	holder.updateType = updateType;

	VBO<T>* vboPtr = vbo.get();

	holder.update = [width, arraySize, size, vboPtr, updateFn]() {
		Buffer<T> buffer(width, arraySize, size);

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
