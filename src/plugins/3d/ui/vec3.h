#pragma once

#include <QDoubleSpinBox>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

class vec3_ui : public possumwood::properties::property<Imath::Vec3<float>, vec3_ui> {
	public:
		vec3_ui();
		virtual ~vec3_ui();

		virtual Imath::Vec3<float> get() const override;
		virtual void set(const Imath::Vec3<float>& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;
		std::vector<QDoubleSpinBox*> m_values;
		std::vector<QMetaObject::Connection> m_connections;
};
