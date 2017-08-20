#include "motion_map.h"

namespace anim {

namespace {

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

}

MotionMap::MotionMap() : m_pixmap(new QGraphicsPixmapItem()) {
	addItem(m_pixmap);
}

void MotionMap::init(const anim::Animation& ax, const anim::Animation& ay) {
	std::vector<float> matrix(ax.frames.size() * ay.frames.size());
	float maxVal = 0.0f;
	float minVal = compare(ax.frames[0], ay.frames[0]);

	for(unsigned a=0; a<ax.frames.size(); ++a)
		for(unsigned b=a; b<ay.frames.size(); ++b) {
			auto& f1 = ax.frames[a];
			auto& f2 = ay.frames[b];

			const float res = compare(f1, f2);
			maxVal = std::max(res, maxVal);
			minVal = std::min(res, minVal);

			matrix[a + b*ax.frames.size()] = res;
			matrix[b + a*ay.frames.size()] = res;
		}

	if(maxVal > 0.0f)
		for(auto& f : matrix)
			f = (f - minVal) / (maxVal - minVal) * 255.0f;

	QImage img = QImage(ax.frames.size(), ay.frames.size(), QImage::Format_RGB32);

	for(unsigned a=0; a<ax.frames.size(); ++a)
		for(unsigned b=0; b<ay.frames.size(); ++b) {
			const float val = matrix[a + b * ax.frames.size()];
			img.setPixel(a, b, qRgb(val, val, val));
		}

	m_pixmap->setPixmap(QPixmap::fromImage(img));
}

std::size_t MotionMap::width() const {
	return m_pixmap->pixmap().width();
}

std::size_t MotionMap::height() const {
	return m_pixmap->pixmap().height();
}

void MotionMap::mouseMoveEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	emit mouseMove(mouseEvent);
}

void MotionMap::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	emit mousePress(mouseEvent);
}

void MotionMap::mouseReleaseEvent(QGraphicsSceneMouseEvent* mouseEvent) {
	emit mouseRelease(mouseEvent);
}

}
