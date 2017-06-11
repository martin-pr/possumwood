#include "filename.h"

#include <QHBoxLayout>

filename_ui::filename_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	m_lineEdit = new QLineEdit();
	layout->addWidget(m_lineEdit, 1);

	m_connection = QObject::connect(
		m_lineEdit,
		&QLineEdit::editingFinished,
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);
}

filename_ui::~filename_ui() {
	QObject::disconnect(m_connection);
}

Filename filename_ui::get() const {
	Filename result(m_value);
	result.setFilename(m_lineEdit->text().toStdString());

	return result;
}

void filename_ui::set(const Filename& value) {
	m_value = value;

	const bool bs = m_lineEdit->blockSignals(true);
	m_lineEdit->setText(m_value.filename().c_str());
	m_lineEdit->blockSignals(bs);
}

QWidget* filename_ui::widget() {
	return m_widget;
}

void filename_ui::onFlagsChanged(unsigned flags) {
	// m_values[a]->setReadOnly(flags & kOutput);
	// m_values[a]->setDisabled((flags & kDirty) || (flags & kDisabled));
}
