#pragma once

#include <QMetaObject>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include <QDialog>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/skeleton.h"

class QComboBox;

class Skeleton : public possumwood::properties::property<anim::Skeleton, Skeleton> {
	public:
		Skeleton();
		virtual ~Skeleton();

		virtual void get(anim::Skeleton& value) const override;
		virtual void set(const anim::Skeleton& value) override;

		virtual QWidget* widget() override;

	private:
		QWidget* m_widget;
		QLabel* m_boneCountLabel;
		QPushButton* m_showDetailsButton;

		QDialog* m_detailsDialog;
		QTreeWidget* m_detailsWidget;

		anim::Skeleton m_value;
};
