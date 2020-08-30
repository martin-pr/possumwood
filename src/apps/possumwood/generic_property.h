#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QLabel>
#include <QToolButton>

class GenericProperty final : public possumwood::properties::property_base {
  public:
	GenericProperty();
	virtual ~GenericProperty();

	virtual QWidget* widget() override;

	virtual void valueToPort(dependency_graph::Port& port) const override final;
	virtual void valueFromPort(dependency_graph::Port& port) override final;

  private:
	void setValue(const std::string& value);

	QWidget* m_widget;

	QToolButton* m_detailButton;
	QLabel* m_label;

	std::string m_value;

	QMetaObject::Connection m_buttonConnection;
};
