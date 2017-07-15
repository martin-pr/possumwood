#pragma once

#include <boost/noncopyable.hpp>

#include <dependency_graph/values.h>

namespace possumwood {

class Drawable : public boost::noncopyable {
	public:
		Drawable(dependency_graph::Values&& vals);
		virtual ~Drawable();

		virtual void draw() = 0;

		/// a static signal for queuing a refresh - used by the Qt UI to actually do the queuing
		static boost::signals2::connection onRefreshQueued(std::function<void()> fn);

	protected:
		dependency_graph::Values& values();
		const dependency_graph::Values& values() const;

		/// queues a refresh (does not refresh immediately, but on next Qt paint event)
		static void refresh();

	private:
		dependency_graph::Values m_vals;
		static boost::signals2::signal<void()> s_refresh;
};

class DrawableFunctor : public Drawable {
	public:
		DrawableFunctor(dependency_graph::Values&& vals, std::function<void(const dependency_graph::Values&)> draw);

		virtual void draw() override;

	private:
		std::function<void(const dependency_graph::Values&)> m_draw;
};

}
