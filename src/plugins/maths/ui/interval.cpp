#include "interval.h"

#include <possumwood_sdk/app.h>

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QStyle>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

interval_ui::interval_ui() {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_slider = new QSlider();
	m_slider->setMinimum(0);
	m_slider->setMaximum(1000);
	m_slider->setOrientation(Qt::Horizontal);
	layout->addWidget(m_slider, 1);

	m_sliderConnection = QObject::connect(m_slider, &QSlider::valueChanged, [this](int value) -> void {
		float norm_val = (float)value / 1000.0f;
		assert(norm_val >= 0.0f && norm_val <= 1.0f);

		if(m_value.type() == possumwood::maths::Interval::kLinear) {
			m_value = m_value.min() + (m_value.max() - m_value.min()) * norm_val;
		}
		else if(m_value.type() == possumwood::maths::Interval::kLog) {
			assert(m_value.min() > 0.0f);

			const float min = log10(m_value.min());
			const float max = log10(m_value.max());

			norm_val = min + norm_val * (max - min);
			norm_val = powf(10.0f, norm_val);

			m_value = norm_val;
		}
		else
			assert(false);

		callValueChangedCallbacks();
	});

	m_detailsButton = new QToolButton();
	m_detailsButton->setIcon(QIcon::fromTheme("preferences-other"));
	layout->addWidget(m_detailsButton);

	m_buttonConnection = QObject::connect(m_detailsButton, &QToolButton::released, [this]() -> void {
		possumwood::maths::Interval current;
		get(current);

		IntervalDialog* dialog = new IntervalDialog(m_widget, current);
		if(dialog->exec() == QDialog::Accepted) {
			// apply the settings
			dialog->updateInterval(current);

			set(current);

			callValueChangedCallbacks();
		}

		dialog->deleteLater();
	});
}

interval_ui::~interval_ui() {
	QObject::disconnect(m_sliderConnection);
}

void interval_ui::get(possumwood::maths::Interval& value) const {
	value = m_value;
}

void interval_ui::set(const possumwood::maths::Interval& value) {
	m_value = value;

	const bool bs = m_slider->blockSignals(true);

	if(m_value.max() == m_value.min())
		m_slider->setValue(500);

	else if(m_value.type() == possumwood::maths::Interval::kLinear) {
		float val = (m_value.value() - m_value.min()) / (m_value.max() - m_value.min());
		val = std::min(std::max(0.0f, val), 1.0f);

		m_slider->setValue(val * 1000.0f);
	}
	else if(m_value.type() == possumwood::maths::Interval::kLog) {
		assert(m_value.min() > 0.0f);

		const float min = log10(m_value.min());
		const float max = log10(m_value.max());

		float val = (log10(m_value.value()) - min) / (max - min);
		val = std::min(std::max(0.0f, val), 1.0f);

		m_slider->setValue(val * 1000.0f);
	}
	else
		assert(false);

	m_slider->blockSignals(bs);

	m_slider->setTracking(m_value.inProgressUpdates());
}

QWidget* interval_ui::widget() {
	return m_widget;
}

void interval_ui::onFlagsChanged(unsigned flags) {
	m_slider->setDisabled(flags & kDisabled);
	m_detailsButton->setDisabled(flags & kDisabled);
}

///////////////////////

IntervalDialog::IntervalDialog(QWidget* parent, const possumwood::maths::Interval& current) : QDialog(parent) {
	setModal(true);
	setWindowTitle("Interval properties");

	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	QFrame* frame = new QFrame();
	frame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	mainLayout->addWidget(frame, 1);

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	mainLayout->addWidget(buttons, 0);

	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	QFormLayout* layout = new QFormLayout(frame);

	m_type = new QComboBox();
	m_type->addItem("Linear");
	m_type->addItem("Logarithmic");
	layout->addRow("Interval type", m_type);
	if(current.type() == possumwood::maths::Interval::kLinear)
		m_type->setCurrentIndex(0);
	else if(current.type() == possumwood::maths::Interval::kLog)
		m_type->setCurrentIndex(1);
	else
		assert(false);

	m_min = new QDoubleSpinBox();
	m_min->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_min->setValue(current.min());
	layout->addRow("Minimum value", m_min);

	m_max = new QDoubleSpinBox();
	m_max->setRange(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	m_max->setValue(current.max());
	layout->addRow("Maximum value", m_max);

	m_update = new QCheckBox();
	m_update->setChecked(current.inProgressUpdates());
	layout->addRow("Update while dragging", m_update);
}

void IntervalDialog::updateInterval(possumwood::maths::Interval& i) {
	if(m_type->currentIndex() == 0)
		i.setType(possumwood::maths::Interval::kLinear);
	else if(m_type->currentIndex() == 1)
		i.setType(possumwood::maths::Interval::kLog);
	else
		assert(false);

	const float current = i.value();

	i.setMin(m_min->value());
	i.setMax(m_max->value());
	i.setInProgressUpdates(m_update->isChecked());

	i.setValue(current);
}
