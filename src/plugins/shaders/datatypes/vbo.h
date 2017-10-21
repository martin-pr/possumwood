#pragma once

#include <vector>
#include <initializer_list>

#include <boost/noncopyable.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

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

		virtual unsigned width() const = 0;
		virtual GLenum type() const = 0;

	private:
		GLuint m_id;
		bool m_initialised;
};

/// encapsulation of a single vertex data buffer
template<typename T>
class VBO : public VBOBase {
	public:
		VBO();

		/// builds a buffer from begin and end iterator - copies the data into a
		///   buffer that can be sent to the GPU. Iterator has to be dereferenceable
		///   to T (i.e., ITERATOR::value type has to be T, or assignable to T)
		template<typename ITERATOR>
		void init(ITERATOR begin, ITERATOR end);

		/// builds a VBO out of an initializer list
		void init(std::initializer_list<T> l);

		~VBO() final;

	protected:
		virtual unsigned width() const override;
		virtual GLenum type() const override;
};

}
