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

#include <ImathVec.h>

#include "bind.h"
#include "node.h"
#include "connected_edge.h"
#include "graph_widget.h"

namespace po = boost::program_options;

using std::cout;
using std::endl;
using std::flush;

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	qt_bind(result, SIGNAL(triggered(bool)), fn);
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

	Node& n1 = scene.addNode("first", QPointF(-50, 20), {{"aaaaa", Port::kInput, Qt::blue}, {"b", Port::kOutput, Qt::red}});
	Node& n2 = scene.addNode("second", QPointF(50, 20), {{"xxxxxxxxxxxxxxxx", Port::kInputOutput, Qt::red}});

	scene.connect(n1.port(1), n2.port(0));

	///

	graph->setContextMenuCallback([&](QPoint p) {
		QMenu menu(graph);

		menu.addAction(makeAction("Add single input node", [&]() {
			QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));
			scene.addNode(makeUniqueNodeName(), pos, {{"input", Port::kInput, randomColor()}});
		}, &menu));

		menu.addAction(makeAction("Add single output node", [&]() {
			QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));
			scene.addNode(makeUniqueNodeName(), pos, {{"output", Port::kOutput, randomColor()}});
		}, &menu));

		menu.addAction(makeAction("Add random more complex node", [&]() {
			QPointF pos = graph->mapToScene(graph->mapFromGlobal(QCursor::pos()));

			Node& node = scene.addNode(makeUniqueNodeName(), pos);

			const unsigned portCount = rand() % 8 + 1;
			for(unsigned p = 0; p < portCount; ++p) {
				std::stringstream name;
				name << "port_" << p;

				const unsigned pi = rand() % 3;

				if(pi == 0)
					node.addPort(Node::PortDefinition{name.str().c_str(), Port::kInput, randomColor()});
				else if(pi == 1)
					node.addPort(Node::PortDefinition{name.str().c_str(), Port::kOutput, randomColor()});
				else
					node.addPort(Node::PortDefinition{name.str().c_str(), Port::kInputOutput, randomColor()});
			}
		}, &menu));

		menu.exec(p);
	});

	graph->setKeyPressCallback([&](const QKeyEvent & event) {
		if(event.key() == Qt::Key_Delete) {
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

	});

	///

	app.exec();

	return 0;
}
