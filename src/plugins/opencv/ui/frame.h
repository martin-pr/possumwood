#pragma once

#include <QLabel>
#include <QMetaObject>

#include <possumwood_sdk/properties/property.h>

#include "opencv/frame.h"

class FrameUI : public possumwood::properties::property<possumwood::opencv::Frame, FrameUI> {
	public:
		FrameUI();
		virtual ~FrameUI();

		virtual void get(possumwood::opencv::Frame& value) const override;
		virtual void set(const possumwood::opencv::Frame& value) override;

		virtual QWidget* widget() override;

	protected:
		// virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;

		QLabel* m_label;
		// QMetaObject::Connection m_valueChangeConnection;

		cv::Mat m_value;
};
