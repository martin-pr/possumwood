#include "generic_property.h"

GenericProperty::GenericProperty() : m_widget(new QLabel()), m_type(typeid(void)) {
	// m_widget->setEnabled(false);

	// allow copying of the content
	m_widget->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// make the label look like a disabled widget, without actually making it disabled (to allow copying of the text)
	QPalette palette = m_widget->palette();
	palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
	m_widget->setPalette(palette);
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

	// show only first row
	const std::string& text = ss.str();
	auto pos = text.find('\n');
	if(pos != std::string::npos)
		m_widget->setText(ss.str().substr(0, pos).c_str());
	else
		m_widget->setText(ss.str().c_str());

	m_widget->setToolTip(ss.str().c_str());
}
