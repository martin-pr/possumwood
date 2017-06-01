#include "main_window.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QHeaderView>

#include <bind.h>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/values.inl>

#include "adaptor.h"
#include "node_data.h"
#include "metadata.h"
#include "io/io.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	node_editor::qt_bind(result, SIGNAL(triggered(bool)), fn);
	return result;
}

}

MainWindow::MainWindow() : QMainWindow(), m_nodeCounter(0) {
	// initialise the graph with some nodes (to be removed)
	{
		auto& add = m_graph.nodes().add(Metadata::instance("addition"), "add1");
		add.setBlindData(NodeData{QPointF(-100, 20)});

		auto& mult = m_graph.nodes().add(Metadata::instance("multiplication"), "mult1");
		mult.setBlindData(NodeData{QPointF(100, 20)});

		add.port(2).connect(mult.port(0));
	}

	// create the main widget, and the main window content
	QWidget* main = new QWidget();
	setCentralWidget(main);

	QHBoxLayout* layout = new QHBoxLayout(main);
	layout->setContentsMargins(0, 0, 0, 0);

	m_viewport = new Viewport();
	layout->addWidget(m_viewport, 1);

	QVBoxLayout* rightLayout = new QVBoxLayout();
	layout->addLayout(rightLayout);
	rightLayout->setContentsMargins(0, 0, 0, 0);

	m_properties = new Properties();
	rightLayout->addWidget(m_properties);

	m_adaptor = new Adaptor(&m_graph);
	rightLayout->addWidget(m_adaptor);

	// connect the selection signal
	m_adaptor->scene().setNodeSelectionCallback(
	[&](std::set<std::reference_wrapper<node_editor::Node>, node_editor::GraphScene::NodeRefComparator> selection) {
		m_properties->show(m_adaptor->selectedNodes());
	}
	);

	// create the context click menu
	m_adaptor->setContextMenuPolicy(Qt::ActionsContextMenu);

	for(auto& m : Metadata::instances()) {
		m_adaptor->addAction(makeAction(("Add " + m.type() + " node").c_str(), [&m, this]() {
			QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(QCursor::pos()));
			m_graph.nodes().add(m, m.type() + "_" + std::to_string(m_nodeCounter++), NodeData{pos});
		}, m_adaptor));
	}

	QAction* separator = new QAction(m_adaptor);
	separator->setSeparator(true);
	m_adaptor->addAction(separator);

	QAction* deleteAction = new QAction("Delete selected items", m_adaptor);
	deleteAction->setShortcut(QKeySequence::Delete);
	m_adaptor->addAction(deleteAction);
	node_editor::qt_bind(deleteAction, SIGNAL(triggered()), [this]() {
		m_adaptor->deleteSelected();
	});

	// drawing callback
	connect(m_viewport, SIGNAL(render(float)), this, SLOT(draw(float)));
}

void MainWindow::draw(float dt) {
	// draw the floor grid
	glBegin(GL_LINES);

	for(int a = -10; a <= 10; ++a) {
		const float c = 0.3f + (float)(a % 10 == 0) * 0.2f;
		glColor3f(c, c, c);

		glVertex3f(a, 0, -10);
		glVertex3f(a, 0, 10);
	}

	for(int a = -10; a <= 10; ++a) {
		const float c = 0.3f + (float)(a % 10 == 0) * 0.2f;
		glColor3f(c, c, c);

		glVertex3f(-10, 0, a);
		glVertex3f(10, 0, a);
	}

	glEnd();

	// draw everything else
	for(auto& n : m_adaptor->graph().nodes()) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		const Metadata& meta = dynamic_cast<const Metadata&>(n.metadata());
		meta.draw(dependency_graph::Values(n));

		glPopAttrib();
	}

}
