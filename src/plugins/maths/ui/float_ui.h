#pragma once

#include <QDoubleSpinBox>
#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

class float_ui : public possumwood::properties::property<float, float_ui> {
	public:
		float_ui();
		virtual ~float_ui();

		virtual void get(float& value) const override;
		virtual void set(const float& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QDoubleSpinBox* m_spinBox;
		QMetaObject::Connection m_valueChangeConnection;
};
