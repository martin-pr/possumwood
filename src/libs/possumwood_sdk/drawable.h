#pragma once

#include <boost/noncopyable.hpp>

#include <dependency_graph/values.h>

namespace possumwood {

class Drawable : public boost::noncopyable {
	public:
		Drawable(dependency_graph::Values&& vals);
		virtual ~Drawable();

		virtual void draw() = 0;

	protected:
		dependency_graph::Values& values();
		const dependency_graph::Values& values() const;

	private:
		dependency_graph::Values m_vals;
};

class DrawableFunctor : public Drawable {
	public:
		DrawableFunctor(dependency_graph::Values&& vals, std::function<void(const dependency_graph::Values&)> draw);

		virtual void draw() override;

	private:
		std::function<void(const dependency_graph::Values&)> m_draw;
};

}
