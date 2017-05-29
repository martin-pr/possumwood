#include "vec3.h"

#include <QtWidgets/QHBoxLayout>

namespace properties {

vec3_ui::vec3_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	m_x = new QDoubleSpinBox(NULL);
	layout->addWidget(m_x);

	m_y = new QDoubleSpinBox(NULL);
	layout->addWidget(m_y);

	m_z = new QDoubleSpinBox(NULL);
	layout->addWidget(m_z);

	m_xChanged = QObject::connect(
		m_x,
		static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_yChanged = QObject::connect(
		m_y,
		static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_zChanged = QObject::connect(
		m_z,
		static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
		[this]() -> void {
			callValueChangedCallbacks();
		}
	);

	m_x->setKeyboardTracking(false);
	m_y->setKeyboardTracking(false);
	m_z->setKeyboardTracking(false);

	m_x->setRange(-1e13, 1e13);
	m_y->setRange(-1e13, 1e13);
	m_z->setRange(-1e13, 1e13);
}

vec3_ui::~vec3_ui() {
	QObject::disconnect(m_xChanged);
	QObject::disconnect(m_yChanged);
	QObject::disconnect(m_zChanged);
}

Imath::Vec3<float> vec3_ui::get() const {
	return Imath::Vec3<float>(m_x->value(), m_y->value(), m_z->value());
}

void vec3_ui::set(const Imath::Vec3<float>& value) {
	if(m_x->value() != value.x)
		m_x->setValue(value.x);
	if(m_y->value() != value.y)
		m_y->setValue(value.y);
	if(m_z->value() != value.z)
		m_z->setValue(value.z);
}

QWidget* vec3_ui::widget() {
	return m_widget;
}

void vec3_ui::onFlagsChanged(unsigned flags) {
	m_x->setReadOnly(flags & kOutput);
	m_x->setDisabled((flags & kDirty) || (flags & kDisabled));
	m_y->setReadOnly(flags & kOutput);
	m_y->setDisabled((flags & kDirty) || (flags & kDisabled));
	m_z->setReadOnly(flags & kOutput);
	m_z->setDisabled((flags & kDirty) || (flags & kDisabled));
}

}

