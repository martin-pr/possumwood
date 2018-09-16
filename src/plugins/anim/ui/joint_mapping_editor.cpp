#include "joint_mapping_editor.h"

#include <QComboBox>
#include <QHeaderView>

#include <ImathEuler.h>

JointMappingEditor::JointMappingEditor() : m_widget(new QTreeWidget()), m_signalsBlocked(false) {
	m_widget->setRootIsDecorated(false);

	m_widget->headerItem()->setText(0, "from");
	m_widget->headerItem()->setText(1, "to");

	m_widget->header()->setStretchLastSection(false);
	m_widget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
	m_widget->header()->setSectionResizeMode(1, QHeaderView::Stretch);
}

JointMappingEditor::~JointMappingEditor() {
}

void JointMappingEditor::get(anim::JointMappingEditorData& value) const {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	value.clear();
	for(int i = 0; i < m_widget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = m_widget->topLevelItem(i);

		QWidget* cbw1 = m_widget->itemWidget(item, 0);
		QComboBox* cb1 = dynamic_cast<QComboBox*>(cbw1);
		assert(cb1 != NULL);

		QWidget* cbw2 = m_widget->itemWidget(item, 1);
		QComboBox* cb2 = dynamic_cast<QComboBox*>(cbw2);
		assert(cb2 != NULL);

		if((cb1->currentIndex() > 0) || cb2->currentIndex() > 0)
			value.add(cb1->currentIndex()-1, cb2->currentIndex()-1);
	}

	m_signalsBlocked = signalsBlockedOrig;
}

void JointMappingEditor::set(const anim::JointMappingEditorData& value) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	// make sure the top level item count matches
	while(value.size() + 1 > (unsigned)m_widget->topLevelItemCount()) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		m_widget->addTopLevelItem(item);

		QComboBox* cb1 = new QComboBox();
		m_widget->setItemWidget(item, 0, cb1);
		QObject::connect(cb1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		                 [this](int) { valueUpdatedSignal(); });

		QComboBox* cb2 = new QComboBox();
		m_widget->setItemWidget(item, 1, cb2);
		QObject::connect(cb2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		                 [this](int) { valueUpdatedSignal(); });
	}
	while(value.size() + 1 < (unsigned)m_widget->topLevelItemCount())
		delete m_widget->takeTopLevelItem(m_widget->topLevelItemCount() - 1);

	// update the selectable items of all combo boxes
	for(int id = 0; id < m_widget->topLevelItemCount(); ++id) {
		QTreeWidgetItem* item = m_widget->topLevelItem(id);

		QWidget* cb1w = m_widget->itemWidget(item, 0);
		QComboBox* cb1 = dynamic_cast<QComboBox*>(cb1w);
		assert(cb1 != NULL);

		QWidget* cb2w = m_widget->itemWidget(item, 1);
		QComboBox* cb2 = dynamic_cast<QComboBox*>(cb2w);
		assert(cb2 != NULL);

		cb1->clear();
		cb1->addItem("-- unassigned --");
		for(auto& b : value.sourceSkeleton())
			cb1->addItem(b.name().c_str());

		cb2->clear();
		cb2->addItem("-- unassigned --");
		for(auto& b : value.targetSkeleton())
			cb2->addItem(b.name().c_str());
	}

	// transfer the data
	{
		int counter = 0;
		for(auto& v : value) {
			QTreeWidgetItem* item = m_widget->topLevelItem(counter++);

			QWidget* cb1w = m_widget->itemWidget(item, 0);
			QComboBox* cb1 = dynamic_cast<QComboBox*>(cb1w);
			assert(cb1 != NULL);
			cb1->setCurrentIndex(v.first + 1);

			QWidget* cb2w = m_widget->itemWidget(item, 1);
			QComboBox* cb2 = dynamic_cast<QComboBox*>(cb2w);
			assert(cb2 != NULL);
			cb2->setCurrentIndex(v.second + 1);
		}

		// last item should always be zeroed
		QTreeWidgetItem* item = m_widget->topLevelItem(counter);

		QWidget* cb1w = m_widget->itemWidget(item, 0);
		QComboBox* cb1 = dynamic_cast<QComboBox*>(cb1w);
		assert(cb1 != NULL);
		cb1->setCurrentIndex(0);

		QWidget* cb2w = m_widget->itemWidget(item, 1);
		QComboBox* cb2 = dynamic_cast<QComboBox*>(cb2w);
		assert(cb2 != NULL);
		cb2->setCurrentIndex(0);
	}

	m_signalsBlocked = signalsBlockedOrig;
}

QWidget* JointMappingEditor::widget() {
	return m_widget;
}

void JointMappingEditor::onFlagsChanged(unsigned flags) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	for(int id = 0; id < m_widget->topLevelItemCount(); ++id) {
		QTreeWidgetItem* item = m_widget->topLevelItem(id);

		QWidget* cb1w = m_widget->itemWidget(item, 0);
		QComboBox* cb1 = dynamic_cast<QComboBox*>(cb1w);
		assert(cb1 != NULL);
		if(cb1->hasFocus())
			m_focusedComboBox = cb1;
		cb1->setDisabled((flags & kDisabled) || (flags & kOutput));
		if(cb1->isEnabled() && m_focusedComboBox == cb1) {
			m_focusedComboBox = NULL;
			cb1->setFocus();
		}

		QWidget* cb2w = m_widget->itemWidget(item, 1);
		QComboBox* cb2 = dynamic_cast<QComboBox*>(cb2w);
		assert(cb2 != NULL);
		if(cb2->hasFocus())
			m_focusedComboBox = cb2;
		cb2->setDisabled((flags & kDisabled) || (flags & kOutput));
		if(cb2->isEnabled() && m_focusedComboBox == cb2) {
			m_focusedComboBox = NULL;
			cb2->setFocus();
		}
	}

	m_signalsBlocked = signalsBlockedOrig;
}

void JointMappingEditor::valueUpdatedSignal() {
	if(!m_signalsBlocked) {
		m_signalsBlocked = true;
		callValueChangedCallbacks();
		m_signalsBlocked = false;
	}
}
