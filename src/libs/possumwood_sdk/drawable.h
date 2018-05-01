#pragma once

#include <boost/noncopyable.hpp>

#include <ImathMatrix.h>

#include <dependency_graph/values.h>
#include <dependency_graph/state.h>

namespace possumwood {

class Drawable : public boost::noncopyable {
	public:
		struct ViewportState {
			unsigned width, height;
			Imath::M44f projection, modelview;
		};

		Drawable(dependency_graph::Values&& vals);
		virtual ~Drawable();

		/// calls draw() method, and processes the return state
		void doDraw(const ViewportState& viewport);

		/// returns current drawing state
		const dependency_graph::State& drawState() const;

		/// a static signal for queuing a refresh - used by the Qt UI to actually do the queuing
		static boost::signals2::connection onRefreshQueued(std::function<void()> fn);

		/// queues a refresh (does not refresh immediately, but on next Qt paint event)
		static void refresh();

	protected:
		virtual dependency_graph::State draw() = 0;

		dependency_graph::Values& values();
		const dependency_graph::Values& values() const;

		const possumwood::Drawable::ViewportState& viewport() const;

	private:
		dependency_graph::Values m_vals;
		possumwood::Drawable::ViewportState m_viewport;
		static boost::signals2::signal<void()> s_refresh;
		dependency_graph::State m_drawState;
};

class DrawableFunctor : public Drawable {
	public:
		DrawableFunctor(dependency_graph::Values&& vals, std::function<dependency_graph::State(const dependency_graph::Values&, const ViewportState&)> draw);

	protected:
		virtual dependency_graph::State draw() override;

	private:
		std::function<dependency_graph::State(const dependency_graph::Values&, const ViewportState&)> m_draw;
};

}
