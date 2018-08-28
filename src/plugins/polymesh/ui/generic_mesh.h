#pragma once

#include <possumwood_sdk/properties/property.h>

#include "generic_polymesh.h"

#include <QMetaObject>
#include <QLabel>

class QComboBox;

class GenericMeshUI : public possumwood::properties::property<possumwood::polymesh::GenericPolymesh, GenericMeshUI> {
	public:
		GenericMeshUI();
		virtual ~GenericMeshUI();

		virtual void get(possumwood::polymesh::GenericPolymesh& value) const override;
		virtual void set(const possumwood::polymesh::GenericPolymesh& value) override;

		virtual QWidget* widget() override;

	private:
		QLabel* m_widget;
};
