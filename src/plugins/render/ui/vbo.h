#pragma once

#include <QPlainTextEdit>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/vertex_data.h"

class VBO : public possumwood::properties::property<std::shared_ptr<const possumwood::VertexData>, VBO> {
	public:
		VBO();
		virtual ~VBO();

		virtual void get(std::shared_ptr<const possumwood::VertexData>& value) const override;
		virtual void set(const std::shared_ptr<const possumwood::VertexData>& value) override;

		virtual QWidget* widget() override;

	private:
		QPlainTextEdit* m_widget;

		std::shared_ptr<const possumwood::VertexData> m_value;
};
