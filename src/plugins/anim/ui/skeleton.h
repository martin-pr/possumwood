#pragma once

#include <QTreeWidget>
#include <QMetaObject>

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
		QTreeWidget* m_widget;
		anim::Skeleton m_value;
};
