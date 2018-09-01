#include "generic_mesh.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialog>
#include <QTreeWidget>
#include <QHeaderView>

#include <dependency_graph/rtti.h>

GenericMeshUI::GenericMeshUI() {
	m_widget = new QWidget();

	QHBoxLayout* layout = new QHBoxLayout(m_widget);
	layout->setContentsMargins(0,0,0,0);

	m_label = new QLabel(m_widget);
	layout->addWidget(m_label, 1);

	m_detailsButton = new QToolButton(m_widget);
	m_detailsButton->setText("...");
	layout->addWidget(m_detailsButton);

	QAbstractButton::connect(m_detailsButton, &QToolButton::pressed, [this]() {
		QDialog* dialog = new QDialog(m_widget);
		dialog->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		dialog->setWindowTitle("Generic mesh details...");

		QVBoxLayout* layout = new QVBoxLayout(dialog);

		QTreeWidget* tree = new QTreeWidget(dialog);
		tree->headerItem()->setText(1, "asfd");
		tree->header()->hide();
		layout->addWidget(tree, 1);

		{
			QTreeWidgetItem* item = new QTreeWidgetItem(tree);
			item->setText(0, "Vertex attributes");
			item->setFirstColumnSpanned(true);

			for(auto& i : m_vertexAttrs) {
				QTreeWidgetItem* elem = new QTreeWidgetItem();
				elem->setText(0, i.first.c_str());
				elem->setText(1, dependency_graph::unmangledName(i.second.c_str()).c_str());

				item->addChild(elem);
			}
		}

		{
			QTreeWidgetItem* item = new QTreeWidgetItem(tree);
			item->setText(0, "Index attributes");
			item->setFirstColumnSpanned(true);

			for(auto& i : m_indexAttrs) {
				QTreeWidgetItem* elem = new QTreeWidgetItem();
				elem->setText(0, i.first.c_str());
				elem->setText(1, dependency_graph::unmangledName(i.second.c_str()).c_str());

				item->addChild(elem);
			}
		}

		{
			QTreeWidgetItem* item = new QTreeWidgetItem(tree);
			item->setText(0, "Polygon attributes");
			item->setFirstColumnSpanned(true);

			for(auto& i : m_polyAttrs) {
				QTreeWidgetItem* elem = new QTreeWidgetItem();
				elem->setText(0, i.first.c_str());
				elem->setText(1, dependency_graph::unmangledName(i.second.c_str()).c_str());

				item->addChild(elem);
			}
		}

		tree->expandAll();

		dialog->adjustSize();
		dialog->show();

		QDialog::connect(dialog, &QDialog::finished, [dialog]() {
			dialog->deleteLater();
		});
	});
}

GenericMeshUI::~GenericMeshUI() {

}

void GenericMeshUI::get(possumwood::polymesh::GenericPolymesh& value) const {
	assert(false);
}

void GenericMeshUI::set(const possumwood::polymesh::GenericPolymesh& value) {
	std::stringstream text;
	text << value.vertices().size() << " vertices, " << value.polygons().size() << " polygons";

	m_label->setText(text.str().c_str());

	m_vertexAttrs.clear();
	for(auto& h : value.vertices().handles())
		m_vertexAttrs.push_back(std::make_pair(h.name(), h.type().name()));

	m_indexAttrs.clear();
	for(auto& h : value.indices().handles())
		m_indexAttrs.push_back(std::make_pair(h.name(), h.type().name()));

	m_polyAttrs.clear();
	for(auto& h : value.polygons().handles())
		m_polyAttrs.push_back(std::make_pair(h.name(), h.type().name()));
}

QWidget* GenericMeshUI::widget() {
	return m_widget;
}
