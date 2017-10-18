#include "uniforms.inl"

#include <limits>

#include <possumwood_sdk/app.h>

namespace possumwood {

Uniforms::Uniforms() : m_currentTime(std::numeric_limits<float>::infinity()) {
}

void Uniforms::use(GLuint programId) const {
	const bool timeUpdate = m_currentTime != possumwood::App::instance().time();
	m_currentTime = possumwood::App::instance().time();

	for(auto& u : m_uniforms) {
		if(u.updateType == kPerDraw || (timeUpdate && u.updateType == kPerFrame))
			u.updateFunctor(const_cast<std::vector<unsigned char>&>(u.data));

		u.useFunctor(programId, u.name, u.data);
	}
}

}
