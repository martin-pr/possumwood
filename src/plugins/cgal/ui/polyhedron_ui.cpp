#include "polyhedron_ui.h"

#include <QFormLayout>

polyhedron_ui::polyhedron_ui() {
	m_widget = new QFrame(NULL);
	m_widget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QFormLayout* layout = new QFormLayout(m_widget);
	layout->setContentsMargins(5, 3, 5, 3);

	m_halfedgeCount = new QLabel("0");
	m_vertexCount = new QLabel("0");
	m_polyCount = new QLabel("0");

	layout->addRow("Halfedge count", m_halfedgeCount);
	layout->addRow("Vertex count", m_vertexCount);
	layout->addRow("Poly count", m_polyCount);
}

polyhedron_ui::~polyhedron_ui() {

}

void polyhedron_ui::get(std::shared_ptr<const possumwood::CGALPolyhedron>& value) const {
	assert(false && "get() on a mesh should never be called - UI change signals are not implemented");
}

void polyhedron_ui::set(const std::shared_ptr<const possumwood::CGALPolyhedron>& value) {
	if(value) {
		m_halfedgeCount->setText(std::to_string(value->num_halfedges()).c_str());
		m_vertexCount->setText(std::to_string(value->num_vertices()).c_str());
		m_polyCount->setText(std::to_string(value->num_faces()).c_str());
	}
	else {
		m_halfedgeCount->setText("(empty)");
		m_vertexCount->setText("(empty)");
		m_polyCount->setText("(empty)");
	}
}

QWidget* polyhedron_ui::widget() {
	return m_widget;
}
