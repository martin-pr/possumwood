#include <possumwood_sdk/node_implementation.h>

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

#include "datatypes/skeleton.h"
#include "datatypes/animation.h"

namespace {

dependency_graph::InAttr<std::shared_ptr<const anim::Animation>> a_inAnim;
dependency_graph::OutAttr<std::shared_ptr<const anim::Animation>> a_outAnim;

float compare(const anim::Skeleton& s1, const anim::Skeleton& s2) {
	if(s1.size() != s2.size() || s1.size() == 0)
		return 0.0f;

	float result = 0.0f;
	for(unsigned bi=1; bi<s1.size();++bi) {
		const auto& b1 = s1[bi];
		const auto& b2 = s2[bi];

		if((b1.tr().rotation ^ b2.tr().rotation) > 0.0f)
			result += (b1.tr().rotation * b2.tr().rotation.inverse()).angle();
		else
			result += (b1.tr().rotation * (-b2.tr().rotation).inverse()).angle();
	}

	return result;
}

class Editor : public possumwood::Editor {
	public:
		Editor() : m_widget(new QGraphicsView()) {
			QGraphicsScene* scene = new QGraphicsScene();
			m_widget->setScene(scene);

			m_pixmap = new QGraphicsPixmapItem();
			scene->addItem(m_pixmap);

			m_lineX = new QGraphicsLineItem();
			scene->addItem(m_lineX);
			m_lineX->setPen(QPen(QColor(255,128,0)));

			m_lineY = new QGraphicsLineItem();
			scene->addItem(m_lineY);
			m_lineY->setPen(QPen(QColor(255,128,0)));

			m_timeConnection = possumwood::App::instance().onTimeChanged([this](float t) {
				timeChanged(t);
			});
			timeChanged(possumwood::App::instance().time());
		}

		virtual ~Editor() {
			m_timeConnection.disconnect();
		}

		virtual QWidget* widget() override {
			return m_widget;
		}

	protected:
		void timeChanged(float t) {
			if(m_animLength > 0.0f) {
				float pos = t / m_animLength * m_pixmap->pixmap().width();
				pos = std::min(pos, (float)m_pixmap->pixmap().width());

				m_lineX->setLine(pos, 0, pos, m_pixmap->pixmap().height());
				m_lineY->setLine(0, pos, m_pixmap->pixmap().width(), pos);

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
				m_animLength = 0.0f;

				std::shared_ptr<const anim::Animation> anim = values().get(a_inAnim);
				if(anim != nullptr) {
					std::vector<float> matrix(anim->frames.size() * anim->frames.size());
					float maxVal = 0.0f;

					for(unsigned a=0; a<anim->frames.size(); ++a)
						for(unsigned b=a; b<anim->frames.size(); ++b) {
							auto& f1 = anim->frames[a];
							auto& f2 = anim->frames[b];

							const float res = compare(f1, f2);
							maxVal = std::max(res, maxVal);
							matrix[a + b*anim->frames.size()] = res;
							matrix[b + a*anim->frames.size()] = res;
						}

					if(maxVal > 0.0f)
						for(auto& f : matrix)
							f /= maxVal / 255.0f;

					QImage img = QImage(anim->frames.size(), anim->frames.size(), QImage::Format_RGB32);

					for(unsigned a=0; a<anim->frames.size(); ++a)
						for(unsigned b=0; b<anim->frames.size(); ++b) {
							const float val = matrix[a + b * anim->frames.size()];
							img.setPixelColor(a, b, QColor(val, val, val));
						}

					pixmap = QPixmap::fromImage(img);

					m_animLength = (float)anim->frames.size() / anim->fps;
				}

				m_pixmap->setPixmap(pixmap);

				timeChanged(possumwood::App::instance().time());
			}
		}

	private:
		QGraphicsView* m_widget;

		QGraphicsPixmapItem* m_pixmap;
		QGraphicsLineItem *m_lineX, *m_lineY;

		boost::signals2::connection m_timeConnection;
		float m_animLength;
};

dependency_graph::State compute(dependency_graph::Values& values) {
	auto& anim = values.get(a_inAnim);
	values.set(a_outAnim, anim);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inAnim, "in_anim");
	meta.addAttribute(a_outAnim, "out_anim");

	meta.addInfluence(a_inAnim, a_outAnim);

	meta.setEditor<Editor>();
	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/periodic", init);

}
