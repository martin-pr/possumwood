#include "log.h"

#include <QHeaderView>

Log::Log(QWidget* parent) : QTreeWidget(parent) {
	QStringList labels;
	labels << "I" << "Log";
	setHeaderLabels(labels);

	setRootIsDecorated(false);

	header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header()->hide();
}

void Log::addMessage(const QIcon& icon, const QString& message) {
	QTreeWidgetItem* item = new QTreeWidgetItem();
	item->setIcon(0, icon);
	item->setText(1, message);

	item->setToolTip(1, message);

	addTopLevelItem(item);

	scrollToBottom();
}
