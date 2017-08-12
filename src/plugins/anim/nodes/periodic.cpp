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
#include <QGraphicsPixmapWidget>

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
		Editor() : m_widget(new QLabel()) {
			m_widget->setScaledContents(true);
			m_widget->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored );
		}

		virtual QWidget* widget() override {
			return m_widget;
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			if(attr == a_inAnim) {
				QPixmap pixmap;

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
				}

				m_widget->setPixmap(pixmap);
			}
		}

	private:
		QLabel* m_widget;
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
