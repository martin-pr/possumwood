#pragma once

#include "vertex_data.h"

#include "vbo.inl"

namespace possumwood {

template <typename T, std::size_t WIDTH>
void VertexData::addVBO(const std::string& name, std::size_t size, const UpdateType& updateType,
                        std::function<void(Buffer<T, WIDTH>&)> updateFn) {
	assert(size > 0);
	assert(m_vbos.empty() || m_vbos[0].size == size);

	std::unique_ptr<VBO<T, WIDTH>> vbo(new VBO<T, WIDTH>(1, size));

	VBOHolder holder;
	holder.name = name;
	holder.glslType = std::string("in ") + GLSLTraits<std::array<T, WIDTH>>::typeString() + " " + name + ";";
	holder.size = size;
	holder.updateType = updateType;
	// holder.data.resize(size * sizeof(T));

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
