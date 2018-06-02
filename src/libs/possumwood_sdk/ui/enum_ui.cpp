#include "enum_ui.h"

enum_ui::enum_ui() {
	m_combobox = new QComboBox(NULL);

	m_changeConnection =
	    QObject::connect(m_combobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	                     [this]() -> void { callValueChangedCallbacks(); });
}

enum_ui::~enum_ui() {
	QObject::disconnect(m_changeConnection);
}

void enum_ui::get(possumwood::Enum& value) const {
	value.setValue(m_combobox->currentText().toStdString());
}

void enum_ui::set(const possumwood::Enum& value) {
	m_value = value;

	const bool bs = m_combobox->blockSignals(true);

	m_combobox->clear();

	for(auto& o : value.options()) {
		m_combobox->addItem(o.first.c_str());
		if(value.value() == o.first)
			m_combobox->setCurrentIndex(m_combobox->count() - 1);
	}

	m_combobox->blockSignals(bs);
}

QWidget* enum_ui::widget() {
	return m_combobox;
}

void enum_ui::onFlagsChanged(unsigned flags) {
	assert((!(flags & kOutput)) && "Enum should never be used as an output.");

	m_combobox->setDisabled((flags & kDirty) || (flags & kDisabled));
}
