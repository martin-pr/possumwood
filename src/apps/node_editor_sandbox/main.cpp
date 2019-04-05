#include <iostream>
#include <string>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <GL/freeglut.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtGui/QKeyEvent>

#include <qt_node_editor/node.h>
#include <qt_node_editor/connected_edge.h>
#include <qt_node_editor/graph_widget.h>

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

using namespace node_editor;

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	QObject::connect(
		result,
		&QAction::triggered,
		[fn](bool) {
			fn();
		}
	);

	return result;
}

QString makeUniqueNodeName() {
	static unsigned s_counter = 0;
	++s_counter;

	return "node_" + QString::number(s_counter);
}

QColor randomColor() {
	static const std::vector<QColor> colors = {QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255)};
	return colors[rand() % colors.size()];
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

	QApplication app(argc, argv);

	// make a window only with a viewport
	QMainWindow win;

	win.showMaximized();

	///

	GraphWidget* graph = new GraphWidget(&win);
	win.setCentralWidget(graph);

	GraphScene& scene = graph->scene();

	Node& n1 = scene.addNode("first", QPointF(-50, 20));
	n1.addPort(Node::PortDefinition{"aaaaa", Port::kInput, Qt::blue});
	n1.addPort(Node::PortDefinition{"b", Port::kOutput, Qt::red});

	Node& n2 = scene.addNode("second", QPointF(50, 20));
	n2.addPort(Node::PortDefinition{"xxxxxxxxxxxxxxxx", Port::kInput, Qt::red});

	scene.connect(n1.port(1), n2.port(0));

	///

	graph->setContextMenuPolicy(Qt::ActionsContextMenu);

	graph->addAction(makeAction("Add single input node", [&]() {
		QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));
		Node& n = scene.addNode(makeUniqueNodeName(), pos);
		n.addPort(Node::PortDefinition{"input", Port::kInput, randomColor()});
	}, NULL));

	graph->addAction(makeAction("Add single output node", [&]() {
		QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));
		Node& n = scene.addNode(makeUniqueNodeName(), pos);
		n.addPort(Node::PortDefinition{"output", Port::kOutput, randomColor()});
	}, NULL));

	graph->addAction(makeAction("Add random more complex node", [&]() {
		QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));

		Node& node = scene.addNode(makeUniqueNodeName(), pos);

		const unsigned portCount = rand() % 8 + 1;
		for(unsigned p = 0; p < portCount; ++p) {
			std::stringstream name;
			name << "port_" << p;

			const unsigned pi = rand() % 2;

			if(pi == 0)
				node.addPort(Node::PortDefinition{name.str().c_str(), Port::kInput, randomColor()});
			else
				node.addPort(Node::PortDefinition{name.str().c_str(), Port::kOutput, randomColor()});
		}
	}, NULL));

	QAction* separator = new QAction(graph);
	separator->setSeparator(true);
	graph->addAction(separator);

	QAction* deleteAction = new QAction("Delete selected items", graph);
	deleteAction->setShortcut(QKeySequence::Delete);
	graph->addAction(deleteAction);
	QObject::connect(
		deleteAction,
		&QAction::triggered,
		[&scene]() {
			{
				unsigned ei = 0;
				while(ei < scene.edgeCount()) {
					ConnectedEdge& e = scene.edge(ei);
					if(e.isSelected())
						scene.disconnect(e);
					else
						++ei;
				}
			}

			{
				unsigned ni = 0;
				while(ni < scene.nodeCount()) {
					Node& n = scene.node(ni);
					if(n.isSelected())
						scene.removeNode(n);
					else
						++ni;
				}
			}
		}
	);

	QObject::connect(&scene, &node_editor::GraphScene::doubleClicked, [](Node* n) {
		if(!n)
			std::cout << "Double clicked outside a node." << std::endl;
		else
			std::cout << "Double clicked node '" << n->name().toStdString() << "'." << std::endl;
	});

	///

	app.exec();

	return 0;
}
