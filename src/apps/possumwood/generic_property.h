#pragma once

#include <QLabel>

#include <possumwood_sdk/properties/property.h>

class GenericProperty : public possumwood::properties::property_base {
	public:
		GenericProperty();
		virtual ~GenericProperty();

		virtual QWidget* widget() override;
		virtual std::type_index type() const override;

		virtual void valueToPort(dependency_graph::Port& port) const override;
		virtual void valueFromPort(dependency_graph::Port& port) override;

	private:
		QLabel* m_widget;
		std::type_index m_type;
};
