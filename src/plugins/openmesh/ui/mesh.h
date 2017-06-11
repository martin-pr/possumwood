#pragma once

#include "openmesh.h"

#include <possumwood_sdk/properties/property.h>

#include <QLabel>
#include <QFrame>

class mesh_ui : public possumwood::properties::property<std::shared_ptr<const Mesh>, mesh_ui> {
	public:
		mesh_ui();
		virtual ~mesh_ui();

		virtual void get(std::shared_ptr<const Mesh>& value) const override;
		virtual void set(const std::shared_ptr<const Mesh>& value) override;

		virtual QWidget* widget() override;

	private:
		QFrame* m_widget;

		QLabel* m_vertexCount;
		QLabel* m_polyCount;
};
