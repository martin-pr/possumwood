#include "error_dialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QLabel>
#include <QFrame>

ErrorDialog::ErrorDialog(const dependency_graph::State& state, QWidget* parent) : QDialog(parent) {
	setWindowTitle("Error...");

	QVBoxLayout* layout = new QVBoxLayout(this);

	QFrame* frame = new QFrame();
	frame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	layout->addWidget(frame, 1);

	QFormLayout* form = new QFormLayout(frame);

	for(auto& i : state) {
		QPixmap pixmap;
		if(i.first == dependency_graph::State::kError)
			pixmap = style()->standardIcon(QStyle::SP_MessageBoxCritical).pixmap(QSize(16, 16));
		else if(i.first == dependency_graph::State::kWarning)
			pixmap = style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(QSize(16, 16));
		else if(i.first == dependency_graph::State::kError)
			pixmap = style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(QSize(16, 16));
		QLabel* icon = new QLabel();
		icon->setPixmap(pixmap);

		QLabel* text = new QLabel();
		text->setWordWrap(true);
		text->setText(i.second.c_str());

		form->addRow(icon, text);
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
	layout->addWidget(buttons, 0);

	connect(buttons, &QDialogButtonBox::accepted, this, &ErrorDialog::accept);
}
