#include "opencv/ui/frame.h"

FrameUI::FrameUI() : m_label(new QLabel(NULL)) {
	// m_valueChangeConnection = QObject::connect(
	// 	m_checkBox,
	// 	&QCheckBox::stateChanged,
	// 	[this]() -> void {
	// 		callValueChangedCallbacks();
	// 	}
	// );
}

FrameUI::~FrameUI() {
	// QObject::disconnect(m_valueChangeConnection);
}

void FrameUI::get(possumwood::opencv::Frame& value) const {
	// value = m_checkBox->isChecked();

	// do nothing, for now
}

void FrameUI::set(const possumwood::opencv::Frame& value) {
	bool block = m_label->blockSignals(true);

	// update the label
	std::stringstream ss;
	ss << value;

	m_label->setText(ss.str().c_str());

	m_label->blockSignals(block);
}

QWidget* FrameUI::widget() {
	return m_label;
}

// void FrameUI::onFlagsChanged(unsigned flags) {
// 	m_checkBox->setDisabled((flags & kOutput) || (flags & kDisabled));
// }
