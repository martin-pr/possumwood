#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QDialog>
#include <QSlider>
#include <QToolButton>

#include "../interval.h"

class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

class interval_ui : public possumwood::properties::property<possumwood::maths::Interval, interval_ui> {
  public:
	interval_ui();
	virtual ~interval_ui();

	virtual void get(possumwood::maths::Interval& value) const override;
	virtual void set(const possumwood::maths::Interval& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	QWidget* m_widget;
	QSlider* m_slider;
	QToolButton* m_detailsButton;

	possumwood::maths::Interval m_value;

	QMetaObject::Connection m_sliderConnection, m_buttonConnection;
};

class IntervalDialog : public QDialog {
	Q_OBJECT

  public:
	IntervalDialog(QWidget* parent, const possumwood::maths::Interval& current);

	// updates the interval value to match options set up by the user
	void updateInterval(possumwood::maths::Interval& i);

  private:
	QComboBox* m_type;
	QDoubleSpinBox* m_min;
	QDoubleSpinBox* m_max;
	QCheckBox* m_update;
};
