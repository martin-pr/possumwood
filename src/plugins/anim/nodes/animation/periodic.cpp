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
		Editor() {
			m_widget = new anim::MotionMap();

			m_lineX = new QGraphicsLineItem();
			m_widget->scene()->addItem(m_lineX);
			m_lineX->setPen(QPen(QColor(255,128,0)));

			m_lineY = new QGraphicsLineItem();
			m_widget->scene()->addItem(m_lineY);
			m_lineY->setPen(QPen(QColor(255,128,0)));

			m_rect = new QGraphicsRectItem();
			m_rect->setRect(0,0,0,0);
			m_rect->setBrush(QColor(255,128,0,32));
			m_rect->setPen(QPen(QColor(255,128,0,255)));
			m_widget->scene()->addItem(m_rect);

			m_timeConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				timeChanged(t);
			});

			QObject::connect(m_widget, &anim::MotionMap::mousePress, [this](QMouseEvent* event) {
				if(event->button() == Qt::LeftButton) {
					const QPointF scenePos = m_widget->mapToScene(event->pos());
					if(scenePos.x() >= 0.0f && scenePos.x() < (float)m_widget->width() &&
						scenePos.y() >= 0.0f && scenePos.y() < (float)m_widget->height()) {

						values().set(a_startFrame, (unsigned)std::min(scenePos.x(), scenePos.y()));
						values().set(a_endFrame, (unsigned)std::max(scenePos.x(), scenePos.y()));
					}
				}
			});

			QObject::connect(m_widget, &anim::MotionMap::mouseMove, [this](QMouseEvent* event) {
				if(event->buttons() & Qt::LeftButton) {
					const QPointF scenePos = m_widget->mapToScene(event->pos());
					if(scenePos.x() >= 0.0f && scenePos.x() < (float)m_widget->width() &&
						scenePos.y() >= 0.0f && scenePos.y() < (float)m_widget->height()) {

						values().set(a_startFrame, (unsigned)std::min(scenePos.x(), scenePos.y()));
						values().set(a_endFrame, (unsigned)std::max(scenePos.x(), scenePos.y()));
					}
				}
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
			const std::pair<float, float> interval = startAndEndFrame(values(), m_widget->width());

			if(m_fps > 0.0f && interval.first != interval.second) {
				float pos = t * m_fps;
				if(pos < (float)((interval.second - interval.first)) * values().get(a_repetitions)) {
					pos = fmodf(pos, fabs(interval.second - interval.first));
					pos += interval.first;
				}
				else
					pos = interval.second;

				pos = std::min(pos, (float)m_widget->width());

				m_lineX->setLine(pos, 0, pos, m_widget->height());
				m_lineY->setLine(0, pos, m_widget->width(), pos);

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
				if(anim != nullptr && !anim->empty()) {
					m_widget->init(*anim);

					m_fps = anim->fps();
				}

				timeChanged(possumwood::App::instance().time());
			}

			if(attr == a_startFrame || attr == a_endFrame || attr == a_inAnim) {
				std::shared_ptr<const anim::Animation> anim = values().get(a_inAnim);
				if(anim != nullptr) {
					std::pair<unsigned, unsigned> interval = startAndEndFrame(values(), anim->size());

					m_rect->setRect(interval.first, interval.first, interval.second - interval.first, interval.second - interval.first);
				}
				else
					m_rect->setRect(0,0,0,0);

				timeChanged(possumwood::App::instance().time());
			}
		}

	private:
		anim::MotionMap* m_widget;

		QGraphicsLineItem *m_lineX, *m_lineY;
		QGraphicsRectItem* m_rect;

		boost::signals2::connection m_timeConnection;
		float m_fps;
};

dependency_graph::State compute(dependency_graph::Values& values) {
	auto& anim = values.get(a_inAnim);

	if(anim != nullptr && anim->size() > 0) {
		std::pair<unsigned, unsigned> interval = startAndEndFrame(values, (unsigned)anim->size());

		// if the interval makes sense
		if(interval.first != interval.second) {
			// make a new animation instance
			std::unique_ptr<anim::Animation> out(new anim::Animation(anim->fps()));

			// root transformation difference between first and last frame =
			//   the "global delta" transformation between periods
			const anim::Transform periodRootTr = anim->frame(interval.second)[0].tr() * anim->frame(interval.first)[0].tr().inverse();

			// the difference frame between first and last frame =
			//   the "local transform difference"
			//   (inverse multiplication order, because how transforms vs quats work)
			anim::Skeleton periotTr = anim->frame(interval.second);
			for(unsigned bi=0;bi<periotTr.size();++bi)
				periotTr[bi].tr() = anim->frame(interval.first)[bi].tr().inverse() * periotTr[bi].tr();

			// iterate over the entire interval
			anim::Transform rootTr;
			const unsigned numFrames = (interval.second - interval.first)*values.get(a_repetitions);
			for(unsigned a=0;a<=numFrames;++a) {
				// compute a frame id inside a single period
				std::size_t frameId = a % (interval.second - interval.first) + interval.first;
				// just a safety when the input values are not correct
				frameId = std::min(frameId, anim->size()-1);

				// get the source frame
				anim::Skeleton fr = anim->frame(frameId);

				// the weight of how much of "delta" should be blended into the "local" frame
				//   -> from 0.5f to -0.5f through each period, so first and last frame end up
				//      the same with minimal adjustments
				const float weight = 0.5f - fmodf((float)a / (float)(interval.second - interval.first), 1.0f);

				// blend-in the local frame
				for(unsigned bi=1;bi<periotTr.size();++bi) {
					// "identity" part of the blend
					Imath::Quatf q = Imath::Quatf::identity() * (1.0f - std::fabs(weight));

					// antipodality handling - make sure the nlerp blend is done
					//   on the same halfsphere in the 4D space
					Imath::Quatf rot = periotTr[bi].tr().rotation;
					if((q ^ rot) < 0.0f)
						rot = -rot;

					// do the NLERP blending (negative weights handled using quat inverse)
					if(weight > 0.0f)
						q = q + periotTr[bi].tr().rotation * weight;
					else
						q = q + periotTr[bi].tr().rotation.inverse() * (-weight);
					q.normalize();
					// and do the translational blending - NEEDS TESTING, THE SIGN MIGHT BE WRONG
					//   (currently don't have anim examples that would highlight the difference)
					Imath::V3f v = periotTr[bi].tr().translation * weight;

					// and add the transform to the bone - reversed multiplication for quats
					fr[bi].tr().rotation = fr[bi].tr().rotation * q;
					fr[bi].tr().translation += v;
				}

				// for each full period, add the global transformation to "advance" the anim
				if(a % (interval.second - interval.first) == 0 && a > 0)
					rootTr = periodRootTr * rootTr;
				fr[0].tr() = rootTr * fr[0].tr();

				// and store the result
				out->addFrame(fr);
			}

			values.set(a_outAnim, std::shared_ptr<const anim::Animation>(out.release()));
		}
		else
			values.set(a_outAnim, std::shared_ptr<const anim::Animation>());
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

possumwood::NodeImplementation s_impl("anim/animation/periodic", init);

}
