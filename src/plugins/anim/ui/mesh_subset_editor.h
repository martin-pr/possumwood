#pragma once

#include <OpenEXR/ImathVec.h>
#include <possumwood_sdk/properties/property.h>

#include <QtCore/QMetaObject>
#include <QtWidgets/QTreeWidget>

#include "datatypes/subset_selection.h"

class QComboBox;

class MeshSubsetEditor : public possumwood::properties::property<anim::SubsetSelection, MeshSubsetEditor> {
  public:
	MeshSubsetEditor();
	virtual ~MeshSubsetEditor();

	virtual void get(anim::SubsetSelection& value) const override;
	virtual void set(const anim::SubsetSelection& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	void valueUpdatedSignal();

	QTreeWidget* m_widget;
	mutable bool m_signalsBlocked;
};
