#pragma once

#include <QDoubleSpinBox>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

class vec2u_ui : public possumwood::properties::property<Imath::Vec2<unsigned>, vec2u_ui> {
	public:
		vec2u_ui();
		virtual ~vec2u_ui();

		virtual void get(Imath::Vec2<unsigned>& value) const override;
		virtual void set(const Imath::Vec2<unsigned>& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;
		std::vector<QSpinBox*> m_values;
		std::vector<QMetaObject::Connection> m_connections;
};
