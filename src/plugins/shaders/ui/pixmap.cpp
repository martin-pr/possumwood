#include "pixmap.h"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QSizePolicy>

Pixmap::Pixmap() {
	m_widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_resolutionLabel = new QLabel();
	layout->addWidget(m_resolutionLabel, 1);

	m_showDetailsButton = new QPushButton("Show details...");
	layout->addWidget(m_showDetailsButton, 0);
	QObject::connect(m_showDetailsButton, &QPushButton::pressed, [this]() { m_detailsDialog->show(); });

	m_detailsDialog = new QDialog(m_widget);
	m_detailsDialog->hide();
	m_detailsDialog->resize(500, 800);
	QHBoxLayout* dialogLayout = new QHBoxLayout(m_detailsDialog);
	dialogLayout->setContentsMargins(0, 0, 0, 0);

	m_detailsWidget = new QLabel();
	dialogLayout->addWidget(m_detailsWidget);
}

Pixmap::~Pixmap() {
}

void Pixmap::get(std::shared_ptr<const QPixmap>& value) const {
	value = m_value;
}

void Pixmap::set(const std::shared_ptr<const QPixmap>& value) {
	if(value == nullptr) {
		m_detailsWidget->setPixmap(QPixmap());
		m_resolutionLabel->setText("No image loaded.");
	}
	else {
		m_detailsWidget->setPixmap(*value);
		m_resolutionLabel->setText((std::to_string(value->width()) + " x " + std::to_string(value->height())).c_str());
	}

	m_value = value;
}

QWidget* Pixmap::widget() {
	return m_widget;
}
