#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QtWidgets/QComboBox>

#include "datatypes/mozaic_type.h"

class QComboBox;

class MozaicTypeUI : public possumwood::properties::property<possumwood::MozaicType, MozaicTypeUI> {
  public:
	MozaicTypeUI();
	virtual ~MozaicTypeUI();

	virtual void get(possumwood::MozaicType& value) const override;
	virtual void set(const possumwood::MozaicType& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	void valueUpdatedSignal();

	QComboBox* m_value;
};
