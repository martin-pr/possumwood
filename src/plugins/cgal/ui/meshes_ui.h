#pragma once

#include <boost/algorithm/string/join.hpp>

#include <QDialog>
#include <QLabel>
#include <QMetaObject>
#include <QPushButton>
#include <QTableWidget>

#include <possumwood_sdk/properties/property.h>

#include "meshes.h"

class QComboBox;

class MeshesUI : public possumwood::properties::property<possumwood::Meshes, MeshesUI> {
  public:
	MeshesUI();
	virtual ~MeshesUI();

	virtual void get(possumwood::Meshes& value) const override;
	virtual void set(const possumwood::Meshes& value) override;

	virtual QWidget* widget() override;

  private:
	QWidget* m_widget;
	QLabel* m_meshCountLabel;
	QPushButton* m_showDetailsButton;

	QDialog* m_detailsDialog;
	QTableWidget* m_detailsWidget;
};
