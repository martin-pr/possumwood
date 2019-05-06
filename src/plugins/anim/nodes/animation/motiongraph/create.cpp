#include <possumwood_sdk/node_implementation.h>

#include <utility>

#include <possumwood_sdk/app.h>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>

#include "datatypes/skeleton.h"
#include "datatypes/animation.h"
#include "datatypes/motion_graph.h"
#include "ui/motion_map.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_inAnim;
dependency_graph::InAttr<unsigned> a_transitionCount, a_transitionLength, a_filterSelftransitions;
dependency_graph::OutAttr<anim::MotionMap> a_mmap;
dependency_graph::OutAttr<anim::MotionGraph> a_mgraph;

class Editor : public possumwood::Editor {
	public:
		Editor() : m_widget(nullptr) {
		}

		virtual QWidget* widget() override {
			if(!m_widget)
				m_widget = new anim::ui::MotionMap();

			return m_widget;
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			QPixmap pixmap;

			const anim::MotionMap& mmap = values().get(a_mmap);

			// make sure widget is initialised
			widget();

			// draw the motion map
			m_widget->init(mmap);

			// and draw the local minima
			for(auto& m : mmap.localMinima()) {
				m_widget->setPixel(m.first-1, m.second, QColor(255, 0, 0));
				m_widget->setPixel(m.first, m.second, QColor(255, 0, 0));
				m_widget->setPixel(m.first+1, m.second, QColor(255, 0, 0));
				m_widget->setPixel(m.first, m.second-1, QColor(255, 0, 0));
				m_widget->setPixel(m.first, m.second+1, QColor(255, 0, 0));
			}

			m_widget->repaint();
		}

	private:
		anim::ui::MotionMap* m_widget;
};

dependency_graph::State compute(dependency_graph::Values& values) {
	// make a motion map
	if(values.isDirty(a_mmap)) {
		::anim::MotionMap mmap(values.get(a_inAnim), ::anim::metric::LocalAngle());

		::anim::filter::LinearTransition lin(values.get(a_transitionLength));
		mmap.filter(lin);

		::anim::filter::IgnoreIdentity ident(values.get(a_filterSelftransitions));
		mmap.filter(ident);

		mmap.computeLocalMinima(values.get(a_transitionCount), values.get(a_transitionLength)/2+1);

		values.set(a_mmap, mmap);
	}

	// make a motion graph out of this
	values.set(a_mgraph, anim::MotionGraph(values.get(a_inAnim), values.get(a_mmap).localMinima()));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim", anim::Animation(24.0f), possumwood::AttrFlags::kVertical);

	meta.addAttribute(a_transitionCount, "transition_count", 50u);
	meta.addAttribute(a_transitionLength, "transition_length", 10u);
	meta.addAttribute(a_filterSelftransitions, "remove_self_transitions", 20u);
	meta.addAttribute(a_mmap, "motion_map", anim::MotionMap(), possumwood::AttrFlags::kHidden);
	meta.addAttribute(a_mgraph, "motion_graph", anim::MotionGraph(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inAnim, a_mmap);
	meta.addInfluence(a_transitionCount, a_mmap);
	meta.addInfluence(a_transitionLength, a_mmap);
	meta.addInfluence(a_filterSelftransitions, a_mmap);

	meta.addInfluence(a_inAnim, a_mgraph);
	meta.addInfluence(a_transitionCount, a_mgraph);
	meta.addInfluence(a_transitionLength, a_mgraph);
	meta.addInfluence(a_filterSelftransitions, a_mgraph);

	meta.setEditor<Editor>();
	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/motiongraph/create", init);

}
