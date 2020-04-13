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

	private:
		QWidget* m_widget;

		QLabel* m_label;

		cv::Mat m_value;
};
