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
#include "ui/motion_map.h"

namespace {

dependency_graph::InAttr<anim::Animation> a_inAnim;
dependency_graph::InAttr<unsigned> a_transitionCount;
dependency_graph::InAttr<unsigned> a_transitionLength;
dependency_graph::InAttr<unsigned> a_test;

class Editor : public possumwood::Editor {
	public:
		Editor() {
			m_widget = new anim::ui::MotionMap();
		}

		virtual QWidget* widget() override {
			return m_widget;
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			QPixmap pixmap;

			anim::Animation anim = values().get(a_inAnim);
			if(!anim.empty()) {
				::anim::MotionMap mmap(anim, ::anim::metric::LocalAngle());

				::anim::filter::LinearTransition lin(values().get(a_transitionLength));
				mmap.filter(lin);

				::anim::filter::IgnoreIdentity ident(values().get(a_transitionLength));
				mmap.filter(ident);

				m_widget->init(mmap);

				auto minima = mmap.localMinima(values().get(a_transitionCount));
				for(auto& m : minima) {
					m_widget->setPixel(m.first-1, m.second, QColor(255, 0, 0));
					m_widget->setPixel(m.first, m.second, QColor(255, 0, 0));
					m_widget->setPixel(m.first+1, m.second, QColor(255, 0, 0));
					m_widget->setPixel(m.first, m.second-1, QColor(255, 0, 0));
					m_widget->setPixel(m.first, m.second+1, QColor(255, 0, 0));
				}
			}

			m_widget->repaint();
		}

	private:
		anim::ui::MotionMap* m_widget;
};

dependency_graph::State compute(dependency_graph::Values& values) {
	// nothing (for now)

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim", anim::Animation(24.0f), possumwood::AttrFlags::kVertical);

	meta.addAttribute(a_test, "test", 50u, possumwood::AttrFlags::kHidden);
	meta.addAttribute(a_transitionCount, "transition_count", 50u);
	meta.addAttribute(a_transitionLength, "transition_length", 10u);

	meta.setEditor<Editor>();
	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/animation/motiongraph", init);

}
