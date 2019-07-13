#include "generic_property.h"

GenericProperty::GenericProperty() : m_widget(new QLabel()), m_type(typeid(void)) {
	// m_widget->setEnabled(false);
	m_widget->setTextInteractionFlags(Qt::TextSelectableByMouse);
}

GenericProperty::~GenericProperty() {
}

QWidget* GenericProperty::widget() {
	return m_widget;
}

std::type_index GenericProperty::type() const {
	return m_type;
}

void GenericProperty::valueToPort(dependency_graph::Port& port) const {
	// nothing
}

void GenericProperty::valueFromPort(dependency_graph::Port& port) {
	m_type = port.type();

	std::stringstream ss;
	ss << port.getData();

	m_widget->setText(ss.str().c_str());
}
