#pragma once

#include <QDoubleSpinBox>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

class vec2_ui : public possumwood::properties::property<Imath::Vec2<float>, vec2_ui> {
	public:
		vec2_ui();
		virtual ~vec2_ui();

		virtual void get(Imath::Vec2<float>& value) const override;
		virtual void set(const Imath::Vec2<float>& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;
		std::vector<QDoubleSpinBox*> m_values;
		std::vector<QMetaObject::Connection> m_connections;
};
