#pragma once

#include <vector>
#include <initializer_list>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

#include "buffer.h"

namespace possumwood {

class VBOBase : public boost::noncopyable {
	public:
		VBOBase();
		virtual ~VBOBase();

		GLuint id() const;

		bool isInitialised() const;

		/// use the VBO - glEnableVertexAttribArray and glVertexAttribPointer calls
		void use(GLint attribLocation) const;

	protected:
		void setInitialised(bool val);

		virtual std::size_t arraySize() const = 0;
		virtual std::size_t width() const = 0;
		virtual GLenum type() const = 0;

	private:
		GLuint m_id;
		bool m_initialised;
};

/// encapsulation of a single vertex data buffer.
/// Type T can be either float or int; WIDTH describes the number of elements per array item
template<typename T>
class VBO : public VBOBase {
	public:
		VBO(std::size_t vertexCount, std::size_t arraySize, std::size_t width);

		/// builds a buffer from begin and end iterator - copies the data into a
		///   buffer that can be sent to the GPU. Iterator has to be dereferenceable
		///   to T (i.e., ITERATOR::value type has to be T, or assignable to T)
		void init(Buffer<T>&);

		~VBO() final;

	protected:
		virtual std::size_t arraySize() const override;
		virtual std::size_t width() const override;
		virtual GLenum type() const override;

	private:
		std::size_t m_vertexCount, m_arraySize, m_width;
};

}
