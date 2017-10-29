#pragma once

#include <QPlainTextEdit>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/uniforms.h"

class Uniforms : public possumwood::properties::property<std::shared_ptr<const possumwood::Uniforms>, Uniforms> {
	public:
		Uniforms();
		virtual ~Uniforms();

		virtual void get(std::shared_ptr<const possumwood::Uniforms>& value) const override;
		virtual void set(const std::shared_ptr<const possumwood::Uniforms>& value) override;

		virtual QWidget* widget() override;

	private:
		QPlainTextEdit* m_widget;

		std::shared_ptr<const possumwood::Uniforms> m_value;
};
