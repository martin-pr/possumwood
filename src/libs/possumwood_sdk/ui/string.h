#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QLineEdit>
#include <QToolButton>

class string_ui : public possumwood::properties::property<std::string, string_ui> {
  public:
	string_ui();
	virtual ~string_ui();

	virtual void get(std::string& value) const override;
	virtual void set(const std::string& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	QLineEdit* m_lineEdit;
	QMetaObject::Connection m_lineEditConnection;
};
