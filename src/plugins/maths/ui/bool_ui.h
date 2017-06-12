#pragma once

#include <QCheckBox>
#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

class bool_ui : public possumwood::properties::property<bool, bool_ui> {
	public:
		bool_ui();
		virtual ~bool_ui();

		virtual void get(bool& value) const override;
		virtual void set(const bool& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QCheckBox* m_checkBox;
		QMetaObject::Connection m_valueChangeConnection;
};
