#include "string.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <possumwood_sdk/app.h>

string_ui::string_ui() {
	m_lineEdit = new QLineEdit();

	m_lineEditConnection = QObject::connect(
		m_lineEdit,
		&QLineEdit::editingFinished,
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

}

string_ui::~string_ui() {
	QObject::disconnect(m_lineEditConnection);
}

void string_ui::get(std::string& value) const {
	value = m_lineEdit->text().toStdString();
}

void string_ui::set(const std::string& value) {
	const bool bs = m_lineEdit->blockSignals(true);
	m_lineEdit->setText(value.c_str());
	m_lineEdit->blockSignals(bs);
}

QWidget* string_ui::widget() {
	return m_lineEdit;
}

void string_ui::onFlagsChanged(unsigned flags) {
	const bool bs = m_lineEdit->blockSignals(true);
	m_lineEdit->setDisabled(flags & kDisabled);
	m_lineEdit->blockSignals(bs);
}
