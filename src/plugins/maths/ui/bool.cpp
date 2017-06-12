#include "bool_ui.h"

bool_ui::bool_ui() : m_checkBox(new QCheckBox(NULL)) {
	m_valueChangeConnection = QObject::connect(
		m_checkBox,
		&QCheckBox::stateChanged,
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);
}

bool_ui::~bool_ui() {
	QObject::disconnect(m_valueChangeConnection);
}

void bool_ui::get(bool& value) const {
	value = m_checkBox->isChecked();
}

void bool_ui::set(const bool& value) {
	bool block = m_checkBox->blockSignals(true);

	if(m_checkBox->isChecked() != value)
		m_checkBox->setChecked(value);

	m_checkBox->blockSignals(block);
}

QWidget* bool_ui::widget() {
	return m_checkBox;
}

void bool_ui::onFlagsChanged(unsigned flags) {
	m_checkBox->setDisabled((flags & kOutput) || (flags & kDirty) || (flags & kDisabled));
}
