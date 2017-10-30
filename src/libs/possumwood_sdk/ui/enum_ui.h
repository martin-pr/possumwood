#pragma once

#include "datatypes/enum.h"

#include <possumwood_sdk/properties/property.h>

#include <QComboBox>

class enum_ui : public possumwood::properties::property<possumwood::Enum, enum_ui> {
  public:
	enum_ui();
	virtual ~enum_ui();

	virtual void get(possumwood::Enum& value) const override;
	virtual void set(const possumwood::Enum& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	QComboBox* m_combobox;

	possumwood::Enum m_value;

	QMetaObject::Connection m_changeConnection;
};
