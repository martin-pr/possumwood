#pragma once

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

namespace possumwood {

class Texture : public boost::noncopyable {
	public:
		enum ChannelOrder {
			kRGB = 0,
			kBGR = 1,
			kGray = 2
		};

		struct Format {
			Format(unsigned char rba = 4, ChannelOrder order = kRGB) : row_byte_align(rba), channel_order(order) {
			}

			unsigned char row_byte_align;
			ChannelOrder channel_order;
		};

		Texture(const unsigned char* data, std::size_t width, std::size_t height, const Format& format = Format());
		Texture(const float* data, std::size_t width, std::size_t height, const Format& format = Format());
		~Texture();

		GLuint id() const;

		void use(GLint attribLocation, GLenum textureUnit) const;

	private:
		GLuint m_id;
};

}
