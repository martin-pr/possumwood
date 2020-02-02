#pragma once

#include <QPlainTextEdit>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/vertex_data.h"

class VBO : public possumwood::properties::property<possumwood::VertexData, VBO> {
	public:
		VBO();
		virtual ~VBO();

		virtual void get(possumwood::VertexData& value) const override;
		virtual void set(const possumwood::VertexData& value) override;

		virtual QWidget* widget() override;

	private:
		QPlainTextEdit* m_widget;

		std::string m_value;
};
