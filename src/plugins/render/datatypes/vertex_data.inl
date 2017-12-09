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

	template<>
	struct VertexDataType<int> {
		static std::string glslType(std::size_t width) {
			switch(width) {
				case 1:	return "int";
				case 2:	return "ivec2" /*"dvec2"*/;
				case 3:	return "ivec3" /*"dvec3"*/;
				case 4:	return "ivec4" /*"dvec4"*/;
			};

			assert(false);
			return "unknown";
		}
	};

	template<typename T>
	std::string makeVBOName(std::size_t width, const std::string& name) {
		std::stringstream ss;
		ss << "in " << VertexDataType<T>::glslType(width) + " " + name << ";";

		return ss.str();
	}
}

template <typename T>
void VertexData::addVBO(const std::string& name, std::size_t size, const UpdateType& updateType,
                        std::function<void(Buffer<typename VBOTraits<T>::element>&)> updateFn) {
	assert(size > 0);
	assert(m_vbos.empty() || m_vbos[0].size == size);

	std::unique_ptr<VBO<T>> vbo(new VBO<T>(size));

	VBOHolder holder;
	holder.name = name;
	holder.glslType = makeVBOName<typename VBOTraits<T>::element>(VBOTraits<T>::width(), name);
	holder.size = size;
	holder.updateType = updateType;

	VBO<T>* vboPtr = vbo.get();

	holder.update = [&holder, size, vboPtr, updateFn]() {
		Buffer<typename VBOTraits<T>::element> buffer(VBOTraits<T>::width(), size);

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
