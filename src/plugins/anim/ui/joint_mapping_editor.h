#pragma once

#include <QTreeWidget>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/joint_mapping_editor_data.h"

class QComboBox;

class JointMappingEditor : public possumwood::properties::property<anim::JointMappingEditorData, JointMappingEditor> {
	public:
		JointMappingEditor();
		virtual ~JointMappingEditor();

		virtual void get(anim::JointMappingEditorData& value) const override;
		virtual void set(const anim::JointMappingEditorData& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		void valueUpdatedSignal();

		QTreeWidget* m_widget;
		mutable bool m_signalsBlocked;

		QComboBox* m_focusedComboBox;
};
