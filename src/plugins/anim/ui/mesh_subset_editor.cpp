#include "mesh_subset_editor.h"

#include <ImathEuler.h>

#include <QComboBox>
#include <QHeaderView>

MeshSubsetEditor::MeshSubsetEditor() : m_widget(new QTreeWidget()), m_signalsBlocked(false) {
	m_widget->setRootIsDecorated(false);

	m_widget->header()->hide();

	QObject::connect(m_widget->model(), &QAbstractItemModel::dataChanged,
	                 [this](const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles) {
		                 valueUpdatedSignal();
	                 });
}

MeshSubsetEditor::~MeshSubsetEditor() {
}

void MeshSubsetEditor::get(anim::SubsetSelection& value) const {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	for(int i = 0; i < m_widget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = m_widget->topLevelItem(i);

		if(item->checkState(0) == Qt::Checked)
			value.select(item->text(0).toStdString());
		else
			value.deselect(item->text(0).toStdString());
	}

	m_signalsBlocked = signalsBlockedOrig;
}

void MeshSubsetEditor::set(const anim::SubsetSelection& value) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	// make sure the top level item count matches
	while(value.options().size() > (unsigned)m_widget->topLevelItemCount()) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		m_widget->addTopLevelItem(item);
	}
	while(value.options().size() < (unsigned)m_widget->topLevelItemCount())
		delete m_widget->takeTopLevelItem(m_widget->topLevelItemCount() - 1);

	// update the values on all items
	{
		auto it = value.begin();
		for(int a = 0; a < m_widget->topLevelItemCount(); ++a) {
			assert(it != value.end());

			QTreeWidgetItem* item = m_widget->topLevelItem(a);
			item->setText(0, it->first.c_str());

			if(!it->second)
				item->setCheckState(0, Qt::Unchecked);
			else
				item->setCheckState(0, Qt::Checked);

			++it;
		}
		assert(it == value.end());
	}

	m_signalsBlocked = signalsBlockedOrig;
}

QWidget* MeshSubsetEditor::widget() {
	return m_widget;
}

void MeshSubsetEditor::onFlagsChanged(unsigned flags) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	m_widget->setDisabled(flags & kDisabled);

	m_signalsBlocked = signalsBlockedOrig;
}

void MeshSubsetEditor::valueUpdatedSignal() {
	if(!m_signalsBlocked) {
		m_signalsBlocked = true;
		callValueChangedCallbacks();
		m_signalsBlocked = false;
	}
}
