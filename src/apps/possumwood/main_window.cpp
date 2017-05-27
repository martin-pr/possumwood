#include "main_window.h"

#include <QHBoxLayout>
#include <QAction>
#include <QHeaderView>

#include <bind.h>
#include <attr.inl>
#include <metadata.inl>
#include <values.inl>

#include "adaptor.h"
#include "node_data.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	node_editor::qt_bind(result, SIGNAL(triggered(bool)), fn);
	return result;
}

const dependency_graph::Metadata& additionNode() {
	static dependency_graph::Metadata s_meta("addition");

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

const dependency_graph::Metadata& multiplicationNode() {
	static dependency_graph::Metadata s_meta("multiplication");

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
	layout->setContentsMargins(0,0,0,0);

	m_adaptor = new Adaptor(&m_graph);
	layout->addWidget(m_adaptor, 1);

	m_properties = new Properties();
	layout->addWidget(m_properties);

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

	QAction* separator = new QAction(m_adaptor);
	separator->setSeparator(true);
	m_adaptor->addAction(separator);

	QAction* deleteAction = new QAction("Delete selected items", m_adaptor);
	deleteAction->setShortcut(QKeySequence::Delete);
	m_adaptor->addAction(deleteAction);
	node_editor::qt_bind(deleteAction, SIGNAL(triggered()), [this]() {
		m_adaptor->deleteSelected();
	});
}
