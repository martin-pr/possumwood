#include "mesh.h"

#include <QFormLayout>

mesh_ui::mesh_ui() {
	m_widget = new QFrame(NULL);
	m_widget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QFormLayout* layout = new QFormLayout(m_widget);
	layout->setContentsMargins(5, 3, 5, 3);

	m_vertexCount = new QLabel("0");
	m_polyCount = new QLabel("0");

	layout->addRow("Vertex count", m_vertexCount);
	layout->addRow("Poly count", m_polyCount);
}

mesh_ui::~mesh_ui() {

}

void mesh_ui::get(std::shared_ptr<const Mesh>& value) const {
	assert(false && "get() on a mesh should never be called - UI change signals are not implemented");
}

void mesh_ui::set(const std::shared_ptr<const Mesh>& value) {
	if(value) {
		m_vertexCount->setText(std::to_string(value->n_vertices()).c_str());
		m_polyCount->setText(std::to_string(value->n_faces()).c_str());
	}
	else {
		m_vertexCount->setText("(empty)");
		m_polyCount->setText("(empty)");
	}
}

QWidget* mesh_ui::widget() {
	return m_widget;
}
