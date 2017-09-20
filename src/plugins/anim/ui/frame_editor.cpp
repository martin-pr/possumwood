#include "frame_editor.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHeaderView>
#include <QLineEdit>

#include <ImathEuler.h>

FrameEditor::FrameEditor() : m_widget(new QTreeWidget()), m_signalsBlocked(false) {
	m_widget->setRootIsDecorated(false);

	m_widget->headerItem()->setText(0, "Item");
	m_widget->headerItem()->setText(1, "rx");
	m_widget->headerItem()->setText(2, "ry");
	m_widget->headerItem()->setText(3, "xz");

	m_widget->header()->setStretchLastSection(false);
	m_widget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
}

FrameEditor::~FrameEditor() {
}

void FrameEditor::get(anim::FrameEditorData& value) const {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	value.clear();
	for(int i = 0; i < m_widget->topLevelItemCount(); ++i) {
		QTreeWidgetItem* item = m_widget->topLevelItem(i);

		QWidget* cbw = m_widget->itemWidget(item, 0);
		QComboBox* cb = dynamic_cast<QComboBox*>(cbw);
		assert(cb != NULL);

		if(cb->currentIndex() > 0) {
			Imath::Eulerf euler;

			QWidget* rxw = m_widget->itemWidget(item, 1);
			QDoubleSpinBox* rx = dynamic_cast<QDoubleSpinBox*>(rxw);
			euler.x = rx->value() / 180.0 * M_PI;

			QWidget* ryw = m_widget->itemWidget(item, 2);
			QDoubleSpinBox* ry = dynamic_cast<QDoubleSpinBox*>(ryw);
			euler.y = ry->value() / 180.0 * M_PI;

			QWidget* rzw = m_widget->itemWidget(item, 3);
			QDoubleSpinBox* rz = dynamic_cast<QDoubleSpinBox*>(rzw);
			euler.z = rz->value() / 180.0 * M_PI;

			value.setTransform(cb->currentIndex() - 1, euler.toQuat());
		}
	}

	m_signalsBlocked = signalsBlockedOrig;
}

void FrameEditor::set(const anim::FrameEditorData& value) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	// make sure the top level item count matches
	while(value.size() + 1 > (unsigned)m_widget->topLevelItemCount()) {
		QTreeWidgetItem* item = new QTreeWidgetItem();
		m_widget->addTopLevelItem(item);

		QComboBox* cb = new QComboBox();
		m_widget->setItemWidget(item, 0, cb);
		QObject::connect(cb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		                 [this](int) { valueUpdatedSignal(); });

		QDoubleSpinBox* rx = new QDoubleSpinBox();
		m_widget->setItemWidget(item, 1, rx);
		rx->setRange(-360, 360);
		QObject::connect(rx, &QDoubleSpinBox::editingFinished, [this, rx]() { valueUpdatedSignal(); });

		QDoubleSpinBox* ry = new QDoubleSpinBox();
		m_widget->setItemWidget(item, 2, ry);
		ry->setRange(-360, 360);
		QObject::connect(ry, &QDoubleSpinBox::editingFinished, [this, ry]() { valueUpdatedSignal(); });

		QDoubleSpinBox* rz = new QDoubleSpinBox();
		m_widget->setItemWidget(item, 3, rz);
		rz->setRange(-360, 360);
		QObject::connect(rz, &QDoubleSpinBox::editingFinished, [this, rz]() { valueUpdatedSignal(); });
	}
	while(value.size() + 1 < (unsigned)m_widget->topLevelItemCount())
		delete m_widget->takeTopLevelItem(m_widget->topLevelItemCount() - 1);

	// update the selectable items of all combo boxes
	for(int id = 0; id < m_widget->topLevelItemCount(); ++id) {
		QTreeWidgetItem* item = m_widget->topLevelItem(id);

		QWidget* cbw = m_widget->itemWidget(item, 0);
		QComboBox* cb = dynamic_cast<QComboBox*>(cbw);
		assert(cb != NULL);

		cb->clear();
		cb->addItem("-- unassigned --");
		for(auto& b : value.skeleton())
			cb->addItem(b.name().c_str());
	}

	// transfer the data
	{
		int counter = 0;
		for(auto& v : value) {
			QTreeWidgetItem* item = m_widget->topLevelItem(counter++);

			QWidget* cbw = m_widget->itemWidget(item, 0);
			QComboBox* cb = dynamic_cast<QComboBox*>(cbw);
			assert(cb != NULL);
			cb->setCurrentIndex(v.first + 1);

			const Imath::Eulerf euler(v.second.rotation.toMatrix33());

			QWidget* rxw = m_widget->itemWidget(item, 1);
			QDoubleSpinBox* rx = dynamic_cast<QDoubleSpinBox*>(rxw);
			rx->setValue(euler.x / M_PI * 180.0);

			QWidget* ryw = m_widget->itemWidget(item, 2);
			QDoubleSpinBox* ry = dynamic_cast<QDoubleSpinBox*>(ryw);
			ry->setValue(euler.y / M_PI * 180.0);

			QWidget* rzw = m_widget->itemWidget(item, 3);
			QDoubleSpinBox* rz = dynamic_cast<QDoubleSpinBox*>(rzw);
			rz->setValue(euler.z / M_PI * 180.0);
		}

		// last item should always be zeroed
		QTreeWidgetItem* item = m_widget->topLevelItem(counter);
		QWidget* cbw = m_widget->itemWidget(item, 0);
		QComboBox* cb = dynamic_cast<QComboBox*>(cbw);
		assert(cb != NULL);
		cb->setCurrentIndex(0);

		QWidget* rxw = m_widget->itemWidget(item, 1);
		QDoubleSpinBox* rx = dynamic_cast<QDoubleSpinBox*>(rxw);
		rx->setValue(0);

		QWidget* ryw = m_widget->itemWidget(item, 2);
		QDoubleSpinBox* ry = dynamic_cast<QDoubleSpinBox*>(ryw);
		ry->setValue(0);

		QWidget* rzw = m_widget->itemWidget(item, 3);
		QDoubleSpinBox* rz = dynamic_cast<QDoubleSpinBox*>(rzw);
		rz->setValue(0);
	}

	m_signalsBlocked = signalsBlockedOrig;
}

QWidget* FrameEditor::widget() {
	return m_widget;
}

void FrameEditor::onFlagsChanged(unsigned flags) {
	const bool signalsBlockedOrig = m_signalsBlocked;
	m_signalsBlocked = true;

	for(int id = 0; id < m_widget->topLevelItemCount(); ++id) {
		QTreeWidgetItem* item = m_widget->topLevelItem(id);

		QWidget* cbw = m_widget->itemWidget(item, 0);
		QComboBox* cb = dynamic_cast<QComboBox*>(cbw);
		assert(cb != NULL);
		if(cb->hasFocus())
			m_focusedComboBox = cb;
		cb->setDisabled((flags & kDirty) || (flags & kDisabled) || (flags & kOutput));
		if(cb->isEnabled() && m_focusedComboBox == cb) {
			m_focusedComboBox = NULL;
			cb->setFocus();
		}

		QWidget* rxw = m_widget->itemWidget(item, 1);
		QDoubleSpinBox* rx = dynamic_cast<QDoubleSpinBox*>(rxw);
		rx->setReadOnly((flags & kDirty) || (flags & kDisabled) || (flags & kOutput));

		QWidget* ryw = m_widget->itemWidget(item, 2);
		QDoubleSpinBox* ry = dynamic_cast<QDoubleSpinBox*>(ryw);
		ry->setReadOnly((flags & kDirty) || (flags & kDisabled) || (flags & kOutput));

		QWidget* rzw = m_widget->itemWidget(item, 3);
		QDoubleSpinBox* rz = dynamic_cast<QDoubleSpinBox*>(rzw);
		rz->setReadOnly((flags & kDirty) || (flags & kDisabled) || (flags & kOutput));
	}

	m_signalsBlocked = signalsBlockedOrig;
}

void FrameEditor::valueUpdatedSignal() {
	if(!m_signalsBlocked) {
		m_signalsBlocked = true;
		callValueChangedCallbacks();
		m_signalsBlocked = false;
	}
}
