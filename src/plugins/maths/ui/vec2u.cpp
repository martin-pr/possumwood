#include "vec2u.h"

#include <QtWidgets/QHBoxLayout>

vec2u_ui::vec2u_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	for(unsigned a=0;a<2;++a) {
		m_values.push_back(new QSpinBox(NULL));
		layout->addWidget(m_values.back());

		m_connections.push_back(QObject::connect(
			m_values.back(),
			static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			[this]() -> void {
				callValueChangedCallbacks();
			}
		));

		m_values.back()->setKeyboardTracking(false);
		m_values.back()->setRange(0, std::numeric_limits<int>::max());
	}
}

vec2u_ui::~vec2u_ui() {
	for(auto& c : m_connections)
		QObject::disconnect(c);
}
void vec2u_ui::get(Imath::Vec2<unsigned>& value) const {
	for(unsigned a=0;a<2;++a)
		value[a] = m_values[a]->value();
}

void vec2u_ui::set(const Imath::Vec2<unsigned>& value) {
	for(auto& v : m_values)
		v->blockSignals(true);

	for(unsigned a=0;a<2;++a)
		if(m_values[a]->value() != (int)value[a])
			m_values[a]->setValue(value[a]);

	for(auto& v : m_values)
		v->blockSignals(false);
}

QWidget* vec2u_ui::widget() {
	return m_widget;
}

void vec2u_ui::onFlagsChanged(unsigned flags) {
	for(unsigned a=0;a<2;++a) {
		m_values[a]->setReadOnly(flags & kOutput);
		m_values[a]->setDisabled(flags & kDisabled);
	}
}
