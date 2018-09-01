#pragma once

#include <possumwood_sdk/properties/property.h>

#include "generic_polymesh.h"

#include <QMetaObject>
#include <QLabel>
#include <QToolButton>

class QComboBox;

class GenericMeshUI : public possumwood::properties::property<possumwood::polymesh::GenericPolymesh, GenericMeshUI> {
	public:
		GenericMeshUI();
		virtual ~GenericMeshUI();

		virtual void get(possumwood::polymesh::GenericPolymesh& value) const override;
		virtual void set(const possumwood::polymesh::GenericPolymesh& value) override;

		virtual QWidget* widget() override;

	private:
		QWidget* m_widget;

		QLabel* m_label;
		QToolButton* m_detailsButton;

		std::vector<std::pair<std::string, std::string>> m_vertexAttrs, m_indexAttrs, m_polyAttrs;
};
