#pragma once

#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/rgb.h"

class QDoubleSpinBox;
class ColourWidget;

class RGB : public possumwood::properties::property<QColor, RGB> {
	public:
		RGB();
		virtual ~RGB();

		virtual void get(QColor& value) const override;
		virtual void set(const QColor& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		void coloursChanged(bool callback = true);

		QWidget* m_widget;

		QDoubleSpinBox *m_r, *m_g, *m_b;
		ColourWidget* m_colour;
};

class ColourWidget : public QWidget {
	public:
		ColourWidget(QWidget* parent = NULL);
		virtual ~ColourWidget();

		void setColour(const QColor& c);

	protected:
		virtual void paintEvent(QPaintEvent *event) override;

	private:
		QColor m_colour;
};
