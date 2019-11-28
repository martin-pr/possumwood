// #include "rgb.h"

// #include <QHeaderView>
// #include <QHBoxLayout>
// #include <QSizePolicy>
// #include <QDoubleSpinBox>
// #include <QLabel>
// #include <QPainter>

// RGB::RGB() {
// 	m_widget = new QWidget();
// 	QHBoxLayout* layout = new QHBoxLayout(m_widget);
// 	layout->setContentsMargins(0, 0, 0, 0);

// 	m_r = new QDoubleSpinBox();
// 	m_r->setMinimum(0);
// 	m_r->setMaximum(1);
// 	m_r->setDecimals(3);
// 	m_r->setValue(0);
// 	m_r->setSingleStep(0.1);
// 	layout->addWidget(m_r, 1);

// 	{
// 		void (QDoubleSpinBox::* fn)(double) = &QDoubleSpinBox::valueChanged;
// 		QWidget::connect(m_r, fn, [this](double) {
// 			coloursChanged();
// 		});
// 	}

// 	m_g = new QDoubleSpinBox();
// 	m_g->setMinimum(0);
// 	m_g->setMaximum(1);
// 	m_g->setDecimals(3);
// 	m_g->setValue(0);
// 	m_g->setSingleStep(0.1);
// 	layout->addWidget(m_g, 1);

// 	{
// 		void (QDoubleSpinBox::* fn)(double) = &QDoubleSpinBox::valueChanged;
// 		QWidget::connect(m_g, fn, [this](double) {
// 			coloursChanged();
// 		});
// 	}

// 	m_b = new QDoubleSpinBox();
// 	m_b->setMinimum(0);
// 	m_b->setMaximum(1);
// 	m_b->setDecimals(3);
// 	m_b->setValue(0);
// 	m_b->setSingleStep(0.1);
// 	layout->addWidget(m_b, 1);

// 	{
// 		void (QDoubleSpinBox::* fn)(double) = &QDoubleSpinBox::valueChanged;
// 		QWidget::connect(m_b, fn, [this](double) {
// 			coloursChanged();
// 		});
// 	}

// 	m_colour = new ColourWidget();
// 	m_colour->setMinimumWidth(10);
// 	layout->addWidget(m_colour, 1);
// }

// RGB::~RGB() {
// }

// void RGB::get(QColor& value) const {
// 	value.setRedF(m_r->value());
// 	value.setGreenF(m_g->value());
// 	value.setBlueF(m_b->value());
// }

// void RGB::set(const QColor& value) {
// 	m_r->setValue(value.redF());
// 	m_g->setValue(value.greenF());
// 	m_b->setValue(value.blueF());

// 	coloursChanged(false);
// }

// QWidget* RGB::widget() {
// 	return m_widget;
// }

// void RGB::onFlagsChanged(unsigned flags) {
// 	m_r->setDisabled(flags & kDisabled || flags & kOutput);
// 	m_g->setDisabled(flags & kDisabled || flags & kOutput);
// 	m_b->setDisabled(flags & kDisabled || flags & kOutput);
// }

// void RGB::coloursChanged(bool callback) {
// 	if(callback)
// 		callValueChangedCallbacks();

// 	QColor col;
// 	get(col);
// 	m_colour->setColour(col);
// }

// /////

// ColourWidget::ColourWidget(QWidget* parent) : QWidget(parent) {
// }

// ColourWidget::~ColourWidget() {
// }

// void ColourWidget::setColour(const QColor& c) {
// 	m_colour = c;

// 	update();
// }

// void ColourWidget::paintEvent(QPaintEvent *event) {
// 	{
// 		QPainter painter(this);

// 		painter.fillRect(rect(), m_colour);
// 	}

// 	QWidget::paintEvent(event);
// }
