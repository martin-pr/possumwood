#pragma once

#include <string>
#include <functional>
#include <vector>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include <QPixmap>

#include <actions/traits.h>
#include <possumwood_sdk/drawable.h>

#include "texture.h"

namespace possumwood {

class Uniforms {
  public:
	/// type of update - static, per drawing, per frame
	enum UpdateType { kStatic = 1, kPerDraw = 2, kPerFrame = 3 };

	Uniforms();

	template <typename T>
	void addUniform(const std::string& name, std::size_t size,
	                const UpdateType& updateType, std::function<void(T*, std::size_t, const ViewportState&)> updateFunctor);

	void addTexture(const std::string& name, const unsigned char* data, std::size_t width, std::size_t height, const Texture::Format& format = Texture::Format());
	void addTexture(const std::string& name, const float* data, std::size_t width, std::size_t height, const Texture::Format& format = Texture::Format());

	dependency_graph::State use(GLuint programId, const ViewportState&) const;

	/// returns the number of uniforms and textures stored in this container
	std::size_t size() const;

	/// returns the GLSL declaration of all values in this container
	std::string glslDeclaration() const;

	/// returns the names of all uniforms that have been registered (todo: change to iterators)
	std::set<std::string> names() const;

	private:
		struct DataBase {
			virtual ~DataBase() {};
			virtual std::unique_ptr<DataBase> clone() const = 0;
		};

		template<typename T>
		struct Data : public DataBase {
			virtual std::unique_ptr<DataBase> clone() const {
				return std::unique_ptr<DataBase>(new Data<T>(*this));
			}

			std::vector<T> data;
		};

		struct UniformHolder {
			UniformHolder() {};
			UniformHolder(UniformHolder&&) = default;

			UniformHolder(const UniformHolder& h) : name(h.name), glslType(h.glslType), updateType(h.updateType), updateFunctor(h.updateFunctor), useFunctor(h.useFunctor) {
				data = h.data->clone();
			}

			std::string name, glslType;
			UpdateType updateType;
			std::unique_ptr<DataBase> data;
			mutable bool initialised = false; // ugly :(

			std::function<void(DataBase&, const ViewportState&)> updateFunctor;
			std::function<dependency_graph::State(GLuint, const std::string&, const DataBase&)> useFunctor;
		};

	std::vector<UniformHolder> m_uniforms;

	struct TextureHolder {
		std::string name, glslType;
		std::shared_ptr<const Texture> texture;
	};

	std::vector<TextureHolder> m_textures;

	mutable float m_currentTime;
};

template <>
struct Traits<std::shared_ptr<const Uniforms>> {
	static constexpr std::array<float, 3> colour() {
		return std::array<float, 3>{{1, 0.8, 1}};
	}
};
}
