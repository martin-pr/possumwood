#include "unsigned_ui.h"

#include <limits>

unsigned_ui::unsigned_ui() : m_spinBox(new QSpinBox(NULL)) {
	m_valueChangeConnection = QObject::connect(
		m_spinBox,
		static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_spinBox->setKeyboardTracking(false);

	m_spinBox->setRange(0, std::numeric_limits<int>::max());
}

unsigned_ui::~unsigned_ui() {
	QObject::disconnect(m_valueChangeConnection);
}

void unsigned_ui::get(unsigned& value) const {
	value = m_spinBox->value();
}

void unsigned_ui::set(const unsigned& value) {
	bool block = m_spinBox->blockSignals(true);

	if((unsigned)m_spinBox->value() != value)
		m_spinBox->setValue(value);

	m_spinBox->blockSignals(block);
}

QWidget* unsigned_ui::widget() {
	return m_spinBox;
}

void unsigned_ui::onFlagsChanged(unsigned flags) {
	m_spinBox->setReadOnly(flags & kOutput);
	m_spinBox->setDisabled(flags & kDisabled);
}
