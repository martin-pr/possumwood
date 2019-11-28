// #pragma once

// #include <QMetaObject>
// #include <QLabel>
// #include <QPushButton>
// #include <QDialog>

// #include <possumwood_sdk/properties/property.h>

// #include "datatypes/pixmap.h"

// class QComboBox;

// class Pixmap : public possumwood::properties::property<std::shared_ptr<const QPixmap>, Pixmap> {
// 	public:
// 		Pixmap();
// 		virtual ~Pixmap();

// 		virtual void get(std::shared_ptr<const QPixmap>& value) const override;
// 		virtual void set(const std::shared_ptr<const QPixmap>& value) override;

// 		virtual QWidget* widget() override;

// 	private:
// 		QWidget* m_widget;
// 		QLabel* m_resolutionLabel;
// 		QPushButton* m_showDetailsButton;

// 		QDialog* m_detailsDialog;
// 		QLabel* m_detailsWidget;

// 		std::shared_ptr<const QPixmap> m_value;
// };
