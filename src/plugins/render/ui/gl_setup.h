#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QComboBox>

#include "datatypes/setup.h"

class GLSetup : public possumwood::properties::property<possumwood::GLSetup, GLSetup> {
  public:
	GLSetup();
	virtual ~GLSetup();

	virtual void get(possumwood::GLSetup& value) const override;
	virtual void set(const possumwood::GLSetup& value) override;

	virtual QWidget* widget() override;

  private:
	virtual void onFlagsChanged(unsigned flags) override;

	QWidget* m_widget;

	QComboBox* m_culling;

	QMetaObject::Connection m_valueChangeConnection;
};
