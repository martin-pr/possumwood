#pragma once

#include <string>
#include <functional>
#include <vector>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include <QPixmap>

#include <possumwood_sdk/traits.h>

#include "texture.h"

namespace possumwood {

class Uniforms {
	public:
		/// type of update - static, per drawing, per frame
		enum UpdateType {
			kStatic = 1,
			kPerDraw = 2,
			kPerFrame = 3
		};

		Uniforms();

		template<typename T>
		void addUniform(const std::string& name, const UpdateType& updateType,
		                std::function<T()> updateFunctor);

		void addTexture(const std::string& name, const QPixmap& pixmap);

		void use(GLuint programId) const;

	private:
		struct UniformHolder {
			std::string name;
			UpdateType updateType;

			std::vector<unsigned char> data;
			std::function<void(std::vector<unsigned char>&)> updateFunctor;
			std::function<void(GLuint, const std::string&,
			                   const std::vector<unsigned char>&)> useFunctor;
		};

		std::vector<UniformHolder> m_uniforms;

		struct TextureHolder {
			std::string name;
			std::shared_ptr<const Texture> texture;
		};

		std::vector<TextureHolder> m_textures;

		mutable float m_currentTime;
};

template<>
struct Traits<std::shared_ptr<const Uniforms>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.8, 1}};
	}

};

}
