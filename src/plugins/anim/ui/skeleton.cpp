#include "skeleton.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>

Skeleton::Skeleton() : m_widget(new QTreeWidget()) {
	m_widget->setRootIsDecorated(false);

	m_widget->header()->hide();
}

Skeleton::~Skeleton() {
}

void Skeleton::get(anim::Skeleton& value) const {
	value = m_value;
}

namespace {

void transfer(const anim::Skeleton::Joint& jnt, QTreeWidgetItem* item) {
	// make sure the number of items is right
	while((unsigned)item->childCount() < jnt.children().size())
		item->addChild(new QTreeWidgetItem());
	while((unsigned)item->childCount() > jnt.children().size()) {
		QTreeWidgetItem* chld = item->takeChild(item->childCount() - 1);
		delete chld;
	}

	// transfer the names, and run recursively
	int index = 0;
	for(auto& j : jnt.children()) {
		item->child(index)->setText(0, j.name().c_str());
		transfer(j, item->child(index));

		++index;
	}
}
}

void Skeleton::set(const anim::Skeleton& value) {
	if(value.empty())
		m_widget->clear();
	else
		transfer(value[0], m_widget->invisibleRootItem());
	m_widget->expandAll();

	m_value = value;
}

QWidget* Skeleton::widget() {
	return m_widget;
}
