#include "generic_mesh.h"

GenericMeshUI::GenericMeshUI() {
	m_widget = new QLabel();
}

GenericMeshUI::~GenericMeshUI() {

}

void GenericMeshUI::get(possumwood::polymesh::GenericPolymesh& value) const {
	assert(false);
}

void GenericMeshUI::set(const possumwood::polymesh::GenericPolymesh& value) {
	std::stringstream text;
	text << value.vertices().size() << " vertices, " << value.polygons().size() << " polygons";

	m_widget->setText(text.str().c_str());
}

QWidget* GenericMeshUI::widget() {
	return m_widget;
}
