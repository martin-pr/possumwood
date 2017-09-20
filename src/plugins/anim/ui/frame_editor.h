#pragma once

#include <QTreeWidget>
#include <QMetaObject>

#include <OpenEXR/ImathVec.h>

#include <possumwood_sdk/properties/property.h>

#include "datatypes/frame_editor_data.h"

class QComboBox;

class FrameEditor : public possumwood::properties::property<anim::FrameEditorData, FrameEditor> {
	public:
		FrameEditor();
		virtual ~FrameEditor();

		virtual void get(anim::FrameEditorData& value) const override;
		virtual void set(const anim::FrameEditorData& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		void valueUpdatedSignal();

		QTreeWidget* m_widget;
		mutable bool m_signalsBlocked;

		QComboBox* m_focusedComboBox;
};
