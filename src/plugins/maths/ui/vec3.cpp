#include "vec3.h"

#include <QtWidgets/QHBoxLayout>

vec3_ui::vec3_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	for(unsigned a=0;a<3;++a) {
		m_values.push_back(new QDoubleSpinBox(NULL));
		layout->addWidget(m_values.back());

		m_connections.push_back(QObject::connect(
			m_values.back(),
			static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
			[this]() -> void {
				callValueChangedCallbacks();
			}
		));

		m_values.back()->setKeyboardTracking(false);
		m_values.back()->setRange(-1e13, 1e13);
	}
}

vec3_ui::~vec3_ui() {
	for(auto& c : m_connections)
		QObject::disconnect(c);
}
void vec3_ui::get(Imath::Vec3<float>& value) const {
	for(unsigned a=0;a<3;++a)
		value[a] = m_values[a]->value();
}

void vec3_ui::set(const Imath::Vec3<float>& value) {
	for(auto& v : m_values)
		v->blockSignals(true);

	for(unsigned a=0;a<3;++a)
		if(m_values[a]->value() != value[a])
			m_values[a]->setValue(value[a]);

	for(auto& v : m_values)
		v->blockSignals(false);
}

QWidget* vec3_ui::widget() {
	return m_widget;
}

void vec3_ui::onFlagsChanged(unsigned flags) {
	for(unsigned a=0;a<3;++a) {
		m_values[a]->setReadOnly(flags & kOutput);
		m_values[a]->setDisabled(flags & kDisabled);
	}
}
