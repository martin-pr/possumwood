#pragma once

#include <boost/noncopyable.hpp>

#include <dependency_graph/values.h>
#include <dependency_graph/state.h>

namespace possumwood {

class Drawable : public boost::noncopyable {
	public:
		Drawable(dependency_graph::Values&& vals);
		virtual ~Drawable();

		/// calls draw() method, and processes the return state
		void doDraw(unsigned width, unsigned height);

		/// returns current drawing state
		const dependency_graph::State& drawState() const;

		/// a static signal for queuing a refresh - used by the Qt UI to actually do the queuing
		static boost::signals2::connection onRefreshQueued(std::function<void()> fn);

	protected:
		virtual dependency_graph::State draw() = 0;

		unsigned width() const;
		unsigned height() const;

		dependency_graph::Values& values();
		const dependency_graph::Values& values() const;

		/// queues a refresh (does not refresh immediately, but on next Qt paint event)
		static void refresh();

	private:
		dependency_graph::Values m_vals;
		static boost::signals2::signal<void()> s_refresh;
		dependency_graph::State m_drawState;

		unsigned m_width, m_height;
};

class DrawableFunctor : public Drawable {
	public:
		DrawableFunctor(dependency_graph::Values&& vals, std::function<dependency_graph::State(const dependency_graph::Values&)> draw);

	protected:
		virtual dependency_graph::State draw() override;

	private:
		std::function<dependency_graph::State(const dependency_graph::Values&)> m_draw;
};

}
