#pragma once

#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/rgb.h"

class QDoubleSpinBox;

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
		QWidget* m_widget;

		QDoubleSpinBox *m_r, *m_g, *m_b;
};
