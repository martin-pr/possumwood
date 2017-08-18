#include <possumwood_sdk/node_implementation.h>

#include <utility>

#include <dependency_graph/values.inl>
#include <dependency_graph/attr.inl>
#include <dependency_graph/datablock.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/node.inl>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/metadata.inl>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>

#include "datatypes/skeleton.h"
#include "datatypes/animation.h"
#include "ui/motion_map.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const anim::Animation>> a_inAnim;
dependency_graph::InAttr<unsigned> a_startFrame;
dependency_graph::InAttr<unsigned> a_endFrame;
dependency_graph::InAttr<unsigned> a_repetitions;

dependency_graph::OutAttr<std::shared_ptr<const anim::Animation>> a_outAnim;

std::pair<unsigned, unsigned> startAndEndFrame(const dependency_graph::Values& vals, unsigned frameCount) {
	unsigned startFrame = vals.get(a_startFrame);
	unsigned endFrame = vals.get(a_endFrame);

	startFrame = std::max(0u, startFrame);
	startFrame = std::min(startFrame, frameCount-1);

	endFrame = std::max(0u, endFrame);
	endFrame = std::min(endFrame, frameCount-1);

	if(startFrame > endFrame)
		std::swap(startFrame, endFrame);

	return std::make_pair(startFrame, endFrame);
}

class Editor : public possumwood::Editor {
	public:
		Editor() : m_widget(new QGraphicsView()) {
			m_scene = new anim::MotionMap();
			m_widget->setScene(m_scene);

			m_lineX = new QGraphicsLineItem();
			m_scene->addItem(m_lineX);
			m_lineX->setPen(QPen(QColor(255,128,0)));

			m_lineY = new QGraphicsLineItem();
			m_scene->addItem(m_lineY);
			m_lineY->setPen(QPen(QColor(255,128,0)));

			m_rect = new QGraphicsRectItem();
			m_rect->setRect(0,0,0,0);
			m_rect->setBrush(QColor(255,128,0,32));
			m_rect->setPen(QPen(QColor(255,128,0,255)));
			m_scene->addItem(m_rect);

			m_timeConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				timeChanged(t);
			});
		}

		virtual ~Editor() {
			m_timeConnection.disconnect();
		}

		virtual QWidget* widget() override {
			return m_widget;
		}

	protected:
		void timeChanged(float t) {
			if(m_fps > 0.0f) {
				const std::pair<float, float> interval = startAndEndFrame(values(), m_scene->width());

				float pos = t * m_fps;
				if(pos < (float)((interval.second - interval.first)) * values().get(a_repetitions)) {
					pos = fmodf(pos, fabs(interval.second - interval.first));
					pos += interval.first;
				}
				else
					pos = interval.second;

				pos = std::min(pos, (float)m_scene->width());

				m_lineX->setLine(pos, 0, pos, m_scene->height());
				m_lineY->setLine(0, pos, m_scene->width(), pos);

				m_lineX->show();
				m_lineY->show();
			}
			else {
				m_lineX->setLine(0,0,0,0);
				m_lineY->setLine(0,0,0,0);

				m_lineX->hide();
				m_lineY->hide();
			}
		}

		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			if(attr == a_inAnim) {
				QPixmap pixmap;
				m_fps = 0.0f;

				std::shared_ptr<const anim::Animation> anim = values().get(a_inAnim);
				if(anim != nullptr && !anim->frames.empty()) {
					m_scene->init(*anim, *anim);

					m_fps = anim->fps;
				}

				timeChanged(possumwood::App::instance().time());
			}

			if(attr == a_startFrame || attr == a_endFrame || attr == a_inAnim) {
				std::shared_ptr<const anim::Animation> anim = values().get(a_inAnim);
				if(anim != nullptr) {
					std::pair<unsigned, unsigned> interval = startAndEndFrame(values(), anim->frames.size());

					m_rect->setRect(interval.first, interval.first, interval.second - interval.first, interval.second - interval.first);
				}
				else
					m_rect->setRect(0,0,0,0);

				timeChanged(possumwood::App::instance().time());
			}
		}

	private:
		QGraphicsView* m_widget;
		anim::MotionMap* m_scene;

		QGraphicsLineItem *m_lineX, *m_lineY;
		QGraphicsRectItem* m_rect;

		boost::signals2::connection m_timeConnection;
		float m_fps;
};

dependency_graph::State compute(dependency_graph::Values& values) {
	auto& anim = values.get(a_inAnim);

	if(anim != nullptr && anim->frames.size() > 0 && anim->base.size() > 0) {
		std::pair<unsigned, unsigned> interval = startAndEndFrame(values, (unsigned)anim->frames.size());

		std::unique_ptr<anim::Animation> out(new anim::Animation());
		out->fps = anim->fps;
		out->base = anim->base;

		const anim::Transform periodRootTr = anim->frames[interval.second][0].tr() * anim->frames[interval.first][0].tr().inverse();

		anim::Skeleton periotTr = anim->frames[interval.second];
		for(unsigned bi=0;bi<periotTr.size();++bi)
			periotTr[bi].tr() = periotTr[bi].tr() * anim->frames[interval.first][bi].tr().inverse();

		anim::Transform rootTr;
		const unsigned numFrames = (interval.second - interval.first)*values.get(a_repetitions);
		for(unsigned a=0;a<=numFrames;++a) {
			std::size_t frameId = a % (interval.second - interval.first) + interval.first;
			frameId = std::min(frameId, anim->frames.size()-1);

			anim::Skeleton fr = anim->frames[frameId];

			const float weight = fmodf((float)a / (float)(interval.second - interval.first), 1.0f) - 0.5f;
			for(unsigned bi=1;bi<periotTr.size();++bi) {
				Imath::Quatf q = (Imath::Quatf::identity() * weight + periotTr[bi].tr().rotation * (1.0f - weight)).normalized();
				Imath::V3f v = periotTr[bi].tr().translation * (1.0f - weight);

				fr[bi].tr().rotation *= q;
				fr[bi].tr().translation += v;
			}

			if(a % (interval.second - interval.first) == 0 && a > 0)
				rootTr = rootTr * periodRootTr;
			fr[0].tr() = rootTr * fr[0].tr();

			out->frames.push_back(fr);
		}

		values.set(a_outAnim, std::shared_ptr<const anim::Animation>(out.release()));
	}
	else
		values.set(a_outAnim, std::shared_ptr<const anim::Animation>());

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim");
	meta.addAttribute(a_startFrame, "start_frame");
	meta.addAttribute(a_endFrame, "end_frame");
	meta.addAttribute(a_repetitions, "repetitions", 2u);
	meta.addAttribute(a_outAnim, "out_anim");

	meta.addInfluence(a_inAnim, a_outAnim);
	meta.addInfluence(a_startFrame, a_outAnim);
	meta.addInfluence(a_endFrame, a_outAnim);
	meta.addInfluence(a_repetitions, a_outAnim);

	meta.setEditor<Editor>();
	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/periodic", init);

}
