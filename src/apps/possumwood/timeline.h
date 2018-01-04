#pragma once

#include <QWidget>

/// A simple interactive Timeline widget
class Timeline : public QWidget {
	Q_OBJECT

	public:
		Timeline(QWidget* parent = NULL);
		virtual ~Timeline();

		void setRange(std::pair<float, float> range);
		const std::pair<float, float>& range() const;

		void setValue(float val);
		float value() const;

		void setTickDistance(unsigned dist);
		unsigned tickDistance() const;

		void setLabelDistance(unsigned dist);
		unsigned labelDistance() const;

	signals:
		void valueChanged(float val);

	protected:
		virtual void paintEvent(QPaintEvent * event);
		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent * event);

	private:
		/// compute a tick position based on the value
		int positionFromValue(float value) const;

		/// compute a value based on a position
		float valueFromPosition(int pos) const;

		struct TickSkip {
			unsigned mantissa;
			int exponent;
		};

		/// compute tick skip based on m_tickDistance
		TickSkip tickSkip(unsigned dist) const;

		float m_dpiScale;

		std::pair<float, float> m_range;
		float m_value;

		unsigned m_tickDistance, m_labelDistance;
};
