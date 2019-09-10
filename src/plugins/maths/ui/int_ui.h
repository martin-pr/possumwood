#pragma once

#include <QSpinBox>
#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

class int_ui : public possumwood::properties::property<int, int_ui> {
	public:
		int_ui();
		virtual ~int_ui();

		virtual void get(int& value) const override;
		virtual void set(const int& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QSpinBox* m_spinBox;
		QMetaObject::Connection m_valueChangeConnection;
};
