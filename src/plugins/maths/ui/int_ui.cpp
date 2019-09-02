#include "int_ui.h"

#include <limits>

int_ui::int_ui() : m_spinBox(new QSpinBox(NULL)) {
	m_valueChangeConnection = QObject::connect(
		m_spinBox,
		static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_spinBox->setKeyboardTracking(false);

	m_spinBox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
}

int_ui::~int_ui() {
	QObject::disconnect(m_valueChangeConnection);
}

void int_ui::get(int& value) const {
	value = m_spinBox->value();
}

void int_ui::set(const int& value) {
	bool block = m_spinBox->blockSignals(true);

	if((int)m_spinBox->value() != value)
		m_spinBox->setValue(value);

	m_spinBox->blockSignals(block);
}

QWidget* int_ui::widget() {
	return m_spinBox;
}

void int_ui::onFlagsChanged(unsigned flags) {
	m_spinBox->setReadOnly(flags & kOutput);
	m_spinBox->setDisabled(flags & kDisabled);
}
