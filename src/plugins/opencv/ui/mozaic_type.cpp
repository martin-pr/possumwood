#include "mozaic_type.h"

MozaicTypeUI::MozaicTypeUI() {
	m_value = new QComboBox();

	m_value->addItem("BG", static_cast<int>(possumwood::MozaicType::BG));
	m_value->addItem("GB", static_cast<int>(possumwood::MozaicType::GB));
	m_value->addItem("RG", static_cast<int>(possumwood::MozaicType::RG));
	m_value->addItem("GR", static_cast<int>(possumwood::MozaicType::GR));

	QObject::connect(m_value, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	                 [this](int) { callValueChangedCallbacks(); });
}

MozaicTypeUI::~MozaicTypeUI() {
}

void MozaicTypeUI::get(possumwood::MozaicType& value) const {
	value = static_cast<possumwood::MozaicType>(m_value->currentIndex());
}

void MozaicTypeUI::set(const possumwood::MozaicType& value) {
	m_value->setCurrentIndex(static_cast<int>(value));
}

QWidget* MozaicTypeUI::widget() {
	return m_value;
}

void MozaicTypeUI::onFlagsChanged(unsigned flags) {
	const bool bs = m_value->blockSignals(true);
	m_value->setDisabled(flags & kDisabled);
	m_value->blockSignals(bs);
}
