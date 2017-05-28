#include "main_window.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QHeaderView>

#include <GL/glut.h>

#include <bind.h>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/values.inl>

#include "adaptor.h"
#include "node_data.h"
#include "metadata.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	node_editor::qt_bind(result, SIGNAL(triggered(bool)), fn);
	return result;
}

const Metadata& additionNode() {
	static Metadata s_meta("addition");

	if(!s_meta.isValid()) {
		// create attributes
		static dependency_graph::InAttr<float> additionInput1, additionInput2;
		static dependency_graph::OutAttr<float> additionOutput;

		// add attributes to the Metadata instance
		s_meta.addAttribute(additionInput1, "input_1");
		s_meta.addAttribute(additionInput2, "input_2");
		s_meta.addAttribute(additionOutput, "output");

		s_meta.addInfluence(additionInput1, additionOutput);
		s_meta.addInfluence(additionInput2, additionOutput);

		s_meta.setCompute([&](dependency_graph::Values & data) {
			const float a = data.get(additionInput1);
			const float b = data.get(additionInput2);

			data.set(additionOutput, a + b);
		});
	}

	return s_meta;
}

const Metadata& multiplicationNode() {
	static Metadata s_meta("multiplication");

	if(!s_meta.isValid()) {
		static dependency_graph::InAttr<float> multiplicationInput1, multiplicationInput2;
		static dependency_graph::OutAttr<float> multiplicationOutput;

		s_meta.addAttribute(multiplicationInput1, "input_1");
		s_meta.addAttribute(multiplicationInput2, "input_2");
		s_meta.addAttribute(multiplicationOutput, "output");

		s_meta.addInfluence(multiplicationInput1, multiplicationOutput);
		s_meta.addInfluence(multiplicationInput2, multiplicationOutput);

		s_meta.setCompute([&](dependency_graph::Values & data) {
			const float a = data.get(multiplicationInput1);
			const float b = data.get(multiplicationInput2);

			data.set(multiplicationOutput, a * b);
		});
	}

	return s_meta;
}

const Metadata& cubeNode() {
	static Metadata s_meta("cube");
	static dependency_graph::InAttr<float> a_x, a_y, a_z, a_w, a_h, a_d;

	if(!s_meta.isValid()) {
		s_meta.addAttribute(a_x, "x");
		s_meta.addAttribute(a_y, "y");
		s_meta.addAttribute(a_z, "z");

		s_meta.addAttribute(a_w, "width", 1.0f);
		s_meta.addAttribute(a_h, "height", 1.0f);
		s_meta.addAttribute(a_d, "depth", 1.0f);

		s_meta.setDraw([&](const dependency_graph::Values & data) {
			const float x = data.get(a_x);
			const float y = data.get(a_y);
			const float z = data.get(a_z);
			const float w = data.get(a_w);
			const float h = data.get(a_h);
			const float d = data.get(a_d);

			glTranslatef(x, y, z);
			glScalef(w, h, d);

			glColor3f(1, 0, 0);
			glutWireCube(0.5f);
		});
	}

	return s_meta;
}

}

MainWindow::MainWindow() : QMainWindow(), m_nodeCounter(0) {
	// initialise the graph with some nodes (to be removed)
	{
		auto& add = m_graph.nodes().add(additionNode(), "add1");
		add.setBlindData(NodeData{QPointF(-100, 20)});

		auto& mult = m_graph.nodes().add(multiplicationNode(), "mult1");
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

	m_adaptor->addAction(makeAction("Add addition node", [this]() {
		QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(QCursor::pos()));
		m_graph.nodes().add(additionNode(), "add_" + std::to_string(m_nodeCounter++), NodeData{pos});
	}, m_adaptor));

	m_adaptor->addAction(makeAction("Add multiplication node", [this]() {
		QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(QCursor::pos()));
		m_graph.nodes().add(multiplicationNode(), "mult_" + std::to_string(m_nodeCounter++), NodeData{pos});
	}, m_adaptor));

	m_adaptor->addAction(makeAction("Add cube node", [this]() {
		QPointF pos = m_adaptor->mapToScene(m_adaptor->mapFromGlobal(QCursor::pos()));
		m_graph.nodes().add(cubeNode(), "cube_" + std::to_string(m_nodeCounter++), NodeData{pos});
	}, m_adaptor));

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
