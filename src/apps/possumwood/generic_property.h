#pragma once

#include <QLabel>
#include <QToolButton>

#include <possumwood_sdk/properties/property.h>

class GenericProperty : public possumwood::properties::property_base {
	public:
		GenericProperty();
		virtual ~GenericProperty();

		virtual QWidget* widget() override;
		virtual std::type_index type() const override;

		virtual void valueToPort(dependency_graph::Port& port) const override;
		virtual void valueFromPort(dependency_graph::Port& port) override;

		void setValue(const std::string& value);

	private:
		QWidget* m_widget;

		QToolButton* m_detailButton;
		QLabel* m_label;

		std::string m_value;
		std::type_index m_type;

		QMetaObject::Connection m_buttonConnection;
};
