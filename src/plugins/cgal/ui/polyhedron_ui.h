#pragma once

#include "cgal.h"

#include <possumwood_sdk/properties/property.h>

#include <QLabel>
#include <QFrame>

class polyhedron_ui : public possumwood::properties::property<std::shared_ptr<const possumwood::CGALPolyhedron>, polyhedron_ui> {
	public:
		polyhedron_ui();
		virtual ~polyhedron_ui();

		virtual void get(std::shared_ptr<const possumwood::CGALPolyhedron>& value) const override;
		virtual void set(const std::shared_ptr<const possumwood::CGALPolyhedron>& value) override;

		virtual QWidget* widget() override;

	private:
		QFrame* m_widget;

		QLabel* m_halfedgeCount;
		QLabel* m_vertexCount;
		QLabel* m_polyCount;
};
