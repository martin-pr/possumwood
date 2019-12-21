#pragma once

#include <QLabel>
#include <QToolButton>

#include <possumwood_sdk/properties/property.h>

class GenericProperty final : public possumwood::properties::property_base {
	public:
		GenericProperty();
		virtual ~GenericProperty();

		virtual QWidget* widget() override;

		virtual void valueToPort(dependency_graph::Port& port) const override final;
		virtual void valueFromPort(dependency_graph::Port& port) override final;

		void setValue(const std::string& value);

	private:
		QWidget* m_widget;

		QToolButton* m_detailButton;
		QLabel* m_label;

		std::string m_value;

		QMetaObject::Connection m_buttonConnection;
};
