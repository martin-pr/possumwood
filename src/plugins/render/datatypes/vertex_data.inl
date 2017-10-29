#pragma once

#include "vertex_data.h"

#include "vbo.inl"

namespace possumwood {

template <typename T>
void VertexData::addVBO(const std::string& name, std::size_t size, const UpdateType& updateType,
                        std::function<void(T* begin, T* end)> updateFn) {
	assert(size > 0);
	assert(m_vbos.empty() || m_vbos[0].size == size);

	std::unique_ptr<VBO<T>> vbo(new VBO<T>());

	VBOHolder holder;
	holder.name = name;
	holder.glslType = std::string("in ") + GLSLTraits<T>::typeString() + " " + name + ";";
	holder.size = size;
	holder.updateType = updateType;
	holder.data.resize(size * sizeof(T));

	T* beginPtr = (T*)&(holder.data[0]);
	T* endPtr = beginPtr + size;
	VBO<T>* vboPtr = vbo.get();

	holder.update = [beginPtr, endPtr, vboPtr, updateFn]() {
		updateFn(beginPtr, endPtr);
		vboPtr->init(beginPtr, endPtr);
	};

	holder.vbo = std::move(vbo);

	// update all buffers that are not per-draw
	if(holder.updateType != kPerDraw)
		holder.update();

	m_vbos.push_back(std::move(holder));
}
}
