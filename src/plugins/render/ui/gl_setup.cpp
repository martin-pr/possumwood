#include "gl_setup.h"

#include <QFormLayout>

GLSetup::GLSetup() : m_widget(new QWidget()) {
	QFormLayout* layout = new QFormLayout(m_widget);

	m_culling = new QComboBox();
	m_culling->addItem("None");
	m_culling->addItem("Counter-clockwise");
	m_culling->addItem("Clockwise");

	layout->addRow("Face culling", m_culling);

	m_valueChangeConnection = QObject::connect(m_culling, (void (QComboBox::*)(int)) & QComboBox::currentIndexChanged,
	                                           [this](int index) -> void { callValueChangedCallbacks(); });
}

GLSetup::~GLSetup() {
	QObject::disconnect(m_valueChangeConnection);
}

void GLSetup::get(possumwood::GLSetup& value) const {
	switch(m_culling->currentIndex()) {
		case 0:
			value.setFaceCulling(possumwood::GLSetup::kNone);
			break;
		case 1:
			value.setFaceCulling(possumwood::GLSetup::kCCW);
			break;
		case 2:
			value.setFaceCulling(possumwood::GLSetup::kCW);
			break;
	}
}

void GLSetup::set(const possumwood::GLSetup& value) {
	switch(value.faceCulling()) {
		case possumwood::GLSetup::kNone:
			m_culling->setCurrentIndex(0);
			break;
		case possumwood::GLSetup::kCCW:
			m_culling->setCurrentIndex(1);
			break;
		case possumwood::GLSetup::kCW:
			m_culling->setCurrentIndex(2);
			break;
	}
}

QWidget* GLSetup::widget() {
	return m_widget;
}

void GLSetup::onFlagsChanged(unsigned flags) {
	m_culling->setDisabled((flags & kOutput) || (flags & kDisabled));
}
