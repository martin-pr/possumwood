#include "generic_property.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyle>
#include <QDialog>
#include <QMainWindow>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QDesktopWidget>

GenericProperty::GenericProperty() : m_type(typeid(void)) {
	m_widget = new QWidget(NULL);

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);


	m_label = new QLabel();
	layout->addWidget(m_label, 1);

	// allow copying of the content
	m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// make the label look like a disabled widget, without actually making it disabled (to allow copying of the text)
	// m_label->setEnabled(false);
	QPalette palette = m_label->palette();
	palette.setColor(QPalette::Text, palette.color(QPalette::Disabled, QPalette::Text));
	m_label->setPalette(palette);


	m_detailButton = new QToolButton();
	m_detailButton->setIcon(m_detailButton->style()->standardIcon(QStyle::SP_MessageBoxInformation));
	layout->addWidget(m_detailButton);

	m_buttonConnection = QObject::connect(
		m_detailButton,
		&QToolButton::released,
		[this]() -> void {
			QDialog* dialog = new QDialog(possumwood::App::instance().mainWindow());
			dialog->resize(QDesktopWidget().availableGeometry().size() * 0.7); // arbitrary "sensible" size of 70% of available space

			QVBoxLayout* layout = new QVBoxLayout(dialog);
			layout->setContentsMargins(0,0,0,0);

			QTextEdit* view = new QTextEdit();
			view->setReadOnly(true);
			view->setText(m_value.c_str());
			layout->addWidget(view, 1);

			QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
			layout->addWidget(buttons, 0);

			QDialog::connect(buttons, &QDialogButtonBox::accepted, [=]() {
				dialog->accept();
				dialog->deleteLater();
			});

			dialog->show();
		}
	);

}

GenericProperty::~GenericProperty() {
}

QWidget* GenericProperty::widget() {
	return m_widget;
}

std::type_index GenericProperty::type() const {
	return m_type;
}

void GenericProperty::valueToPort(dependency_graph::Port& port) const {
	// nothing
}

void GenericProperty::valueFromPort(dependency_graph::Port& port) {
	m_type = port.type();

	std::stringstream ss;

	try {
		ss << port.getData();
	}
	catch(const std::exception& ex) {
		ss << ex.what();
	}

	setValue(ss.str());
}

void GenericProperty::setValue(const std::string& value) {
	// show only first row
	m_value = value;
	auto pos = m_value.find('\n');
	if(pos != std::string::npos)
		m_label->setText(value.substr(0, pos).c_str());
	else
		m_label->setText(value.c_str());

	m_widget->setToolTip(value.c_str());
}
