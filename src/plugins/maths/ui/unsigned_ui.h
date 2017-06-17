#pragma once

#include <QSpinBox>
#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

class unsigned_ui : public possumwood::properties::property<unsigned, unsigned_ui> {
	public:
		unsigned_ui();
		virtual ~unsigned_ui();

		virtual void get(unsigned& value) const override;
		virtual void set(const unsigned& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QSpinBox* m_spinBox;
		QMetaObject::Connection m_valueChangeConnection;
};
