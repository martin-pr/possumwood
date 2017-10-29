#pragma once

#include "uniforms.h"

#include <cassert>

#include "glsl_traits.h"

namespace possumwood {

template<typename T>
void Uniforms::addUniform(const std::string& name, const UpdateType& updateType,
                          std::function<T()> updateFunctor) {
	UniformHolder uniform;

	uniform.name = name;
	uniform.glslType = std::string("uniform ") + GLSLTraits<T>::typeString() + " " + name + ";";
	uniform.updateType = updateType;

	uniform.updateFunctor =
		[updateFunctor](std::vector<unsigned char>& raw) {
			if(raw.empty())
				raw.resize(sizeof(T));

			T* data = (T*)(&raw[0]);
			*data = updateFunctor();
		};

	uniform.useFunctor =
		[](GLuint programId, const std::string& name, const std::vector<unsigned char>& raw) {
			GLint attr = glGetUniformLocation(programId, name.c_str());
			if(attr >= 0) {
				assert(raw.size() == sizeof(T));
				const T* data = (const T*)(&raw[0]);

				GLSLTraits<T>::applyUniform(attr, *data);
			}
		};

	if(updateType != kPerDraw)
		uniform.updateFunctor(uniform.data);

	m_uniforms.push_back(uniform);
}

}
