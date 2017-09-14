#pragma once

#include <QTreeWidget>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/skin_mapping_editor_data.h"

class QComboBox;

class SkinMappingEditor : public possumwood::properties::property<anim::SkinMappingEditorData, SkinMappingEditor> {
	public:
		SkinMappingEditor();
		virtual ~SkinMappingEditor();

		virtual void get(anim::SkinMappingEditorData& value) const override;
		virtual void set(const anim::SkinMappingEditorData& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		void valueUpdatedSignal();

		QTreeWidget* m_widget;
		mutable bool m_signalsBlocked;

		QComboBox* m_focusedComboBox;
};
