#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QHBoxLayout>

#include <ImathVec.h>

#include "bind.h"
#include "node.h"
#include "connected_edge.h"
#include "graph_widget.h"
#include "graph.h"
#include "node.inl"
#include "datablock.inl"
#include "node_data.h"
#include "metadata.inl"
#include "attr.inl"
#include "adaptor.h"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

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

		std::function<void(dependency_graph::Datablock&)> additionCompute = [&](dependency_graph::Datablock & data) {
			const float a = data.get(additionInput1);
			const float b = data.get(additionInput2);

			data.set(additionOutput, a + b);
		};
		s_meta.setCompute(additionCompute);
	}

	return s_meta;
}

const dependency_graph::Metadata& multiplicationNode() {
	static dependency_graph::Metadata s_meta("multiplication");

	if(!s_meta.isValid()) {
		static dependency_graph::InAttr<float> multiplicationInput1, multiplicationInput2;
		static dependency_graph::OutAttr<float> multiplicationOutput;
		std::function<void(dependency_graph::Datablock&)> multiplicationCompute = [&](dependency_graph::Datablock & data) {
			const float a = data.get(multiplicationInput1);
			const float b = data.get(multiplicationInput2);

			data.set(multiplicationOutput, a * b);
		};

		s_meta.addAttribute(multiplicationInput1, "input_1");
		s_meta.addAttribute(multiplicationInput2, "input_2");
		s_meta.addAttribute(multiplicationOutput, "output");

		s_meta.addInfluence(multiplicationInput1, multiplicationOutput);
		s_meta.addInfluence(multiplicationInput2, multiplicationOutput);

		s_meta.setCompute(multiplicationCompute);
	}

	return s_meta;
}

}

int main(int argc, char* argv[]) {
	// // Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
	;

	// process the options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help")) {
		cout << desc << "\n";
		return 1;
	}

	///////////////////////////////

	dependency_graph::Graph dg;

	{
		auto& add = dg.nodes().add(additionNode(), "add1");
		add.setBlindData(NodeData{QPointF(-100, 20)});

		auto& mult = dg.nodes().add(multiplicationNode(), "mult1");
		mult.setBlindData(NodeData{QPointF(100, 20)});

		add.port(2).connect(mult.port(0));
	}

	///////////////////////////////

	QApplication app(argc, argv);

	// make a window only with a viewport
	QMainWindow win;

	win.showMaximized();

	///

	QWidget* main = new QWidget();
	win.setCentralWidget(main);

	QHBoxLayout* layout = new QHBoxLayout(main);
	layout->setContentsMargins(0,0,0,0);

	Adaptor* g1 = new Adaptor(&dg);
	layout->addWidget(g1);

	Adaptor* g2 = new Adaptor(&dg);
	layout->addWidget(g2);

	///

	unsigned nodeCounter = 0;

	g1->setContextMenuPolicy(Qt::ActionsContextMenu);

	g1->addAction(makeAction("Add addition node", [&]() {
		QPointF pos = g1->mapToScene(g1->mapFromGlobal(QCursor::pos()));
		dg.nodes().add(additionNode(), "add_" + std::to_string(nodeCounter++), NodeData{pos});
	}, g1));

	g1->addAction(makeAction("Add multiplication node", [&]() {
		QPointF pos = g1->mapToScene(g1->mapFromGlobal(QCursor::pos()));
		dg.nodes().add(multiplicationNode(), "mult_" + std::to_string(nodeCounter++), NodeData{pos});
	}, g1));

	QAction* separator = new QAction(g1);
	separator->setSeparator(true);
	g1->addAction(separator);

	QAction* deleteAction = new QAction("Delete selected items", g1);
	deleteAction->setShortcut(QKeySequence::Delete);
	g1->addAction(deleteAction);
	node_editor::qt_bind(deleteAction, SIGNAL(triggered()), [&]() {
		g1->deleteSelected();
	});

	///

	app.exec();

	return 0;
}
