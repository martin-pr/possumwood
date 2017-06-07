#pragma once

#include <QDoubleSpinBox>
#include <QMetaObject>

#include <possumwood/properties/property.h>

namespace properties {

class float_ui : public property<float, float_ui> {
	public:
		float_ui();
		virtual ~float_ui();

		virtual float get() const override;
		virtual void set(const float& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QDoubleSpinBox* m_spinBox;
		QMetaObject::Connection m_valueChangeConnection;
};

}
