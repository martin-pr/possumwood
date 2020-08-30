#include "skeleton.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QSizePolicy>

Skeleton::Skeleton() {
	m_widget = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0, 0, 0, 0);

	m_boneCountLabel = new QLabel();
	layout->addWidget(m_boneCountLabel, 1);

	m_showDetailsButton = new QPushButton("Show details...");
	layout->addWidget(m_showDetailsButton, 0);
	QObject::connect(m_showDetailsButton, &QPushButton::pressed, [this]() { m_detailsDialog->show(); });

	m_detailsDialog = new QDialog(m_widget);
	m_detailsDialog->hide();
	m_detailsDialog->resize(500, 800);
	QHBoxLayout* dialogLayout = new QHBoxLayout(m_detailsDialog);
	dialogLayout->setContentsMargins(0, 0, 0, 0);

	m_detailsWidget = new QTreeWidget();
	dialogLayout->addWidget(m_detailsWidget);
	m_detailsWidget->header()->hide();
}

Skeleton::~Skeleton() {
}

void Skeleton::get(anim::Skeleton& value) const {
	value = m_value;
}

namespace {

void transfer(const anim::Skeleton::Joint& jnt, QTreeWidgetItem* item) {
	// transfer the name first
	item->setText(0, jnt.name().c_str());

	// make sure the number of items is right
	while((unsigned)item->childCount() < jnt.children().size())
		item->addChild(new QTreeWidgetItem());
	while((unsigned)item->childCount() > jnt.children().size()) {
		QTreeWidgetItem* chld = item->takeChild(item->childCount() - 1);
		delete chld;
	}

	// and run recursively
	int index = 0;
	for(auto& j : jnt.children())
		transfer(j, item->child(index++));
}
}  // namespace

void Skeleton::set(const anim::Skeleton& value) {
	if(value.empty())
		m_detailsWidget->clear();
	else {
		if(m_detailsWidget->topLevelItemCount() == 0)
			m_detailsWidget->addTopLevelItem(new QTreeWidgetItem());

		transfer(value[0], m_detailsWidget->topLevelItem(0));
	}
	m_detailsWidget->expandAll();

	m_boneCountLabel->setText((std::to_string(value.size()) + " bone(s)").c_str());

	m_value = value;
}

QWidget* Skeleton::widget() {
	return m_widget;
}
