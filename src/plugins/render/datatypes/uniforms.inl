#pragma once

#include "uniforms.h"

#include <cassert>

#include "glsl_traits.h"

namespace possumwood {

template <typename T>
void Uniforms::addUniform(
    const std::string& name, std::size_t size, const UpdateType& updateType,
    std::function<void(T*, std::size_t, const ViewportState&)> updateFunctor) {
	std::unique_ptr<UniformHolder> uniform(new UniformHolder());

	uniform->name = name;
	uniform->glslType = std::string("uniform ") + GLSLTraits<T>::typeString() + " " + name;
	if(size > 1)
		uniform->glslType += "[" + std::to_string(size) + "];";
	else
		uniform->glslType += ";";

	uniform->updateType = updateType;

	{
		std::unique_ptr<Data<T>> data(new Data<T>());
		data->data.resize(size);
		uniform->data = std::move(data);
	}

	uniform->updateFunctor = [updateFunctor, size](DataBase& baseData,
	                                              const ViewportState& vs) {
		Data<T>& data = dynamic_cast<Data<T>&>(baseData);
		assert(data.data.size() == size);

		updateFunctor(&(data.data[0]), size, vs);
	};

	uniform->useFunctor = [size](GLuint programId, const std::string& name,
	                            const DataBase& baseData) -> dependency_graph::State {
		dependency_graph::State state;

		const Data<T>& data = dynamic_cast<const Data<T>&>(baseData);
		assert(data.data.size() == size);

		GLint attr = glGetUniformLocation(programId, name.c_str());
		if(attr >= 0)
			GLSLTraits<T>::applyUniform(attr, size, &(data.data[0]));
		// else
		// 	state.addWarning("Uniform '" + name + "' cannot be mapped to an attribute location - not used in any of the programs?");

		return state;
	};

	// if(updateType != kPerDraw)
	// 	uniform->updateFunctor(*uniform->data, vs);

	m_uniforms.push_back(std::shared_ptr<const UniformHolder>(uniform.release()));
}
}
