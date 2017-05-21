#include "float_ui.h"

namespace properties {

float_ui::float_ui() : m_spinBox(new QDoubleSpinBox(NULL)) {
	m_valueChangeConnection = QObject::connect(
		m_spinBox,
		&QDoubleSpinBox::editingFinished,
		// static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);
}

float_ui::~float_ui() {
	QObject::disconnect(m_valueChangeConnection);
}

float float_ui::get() const {
	return m_spinBox->value();
}

void float_ui::set(const float& value) {
	if(m_spinBox->value() != value)
		m_spinBox->setValue(value);
}

QWidget* float_ui::widget() {
	return m_spinBox;
}

void float_ui::onFlagsChanged(unsigned flags) {
	m_spinBox->setReadOnly(flags & kOutput);
	m_spinBox->setDisabled((flags & kDirty) || (flags & kDisabled));
}

}
