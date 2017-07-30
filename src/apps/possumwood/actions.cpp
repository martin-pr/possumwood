#include "actions.h"

#include <QApplication>
#include <QClipboard>

#include <dependency_graph/io/graph.h>

#include <possumwood_sdk/app.h>

void Actions::createNode(const dependency_graph::Metadata& meta, const std::string& name, const possumwood::NodeData& data) {
	possumwood::App::instance().graph().nodes().add(meta, name, data);
}

void Actions::cut(const dependency_graph::Selection& selection) {
	// trigger the copy action first
	copy(selection);

	// and delete selection
	remove(selection);
}

void Actions::copy(const dependency_graph::Selection& selection) {
	// convert the selection to JSON string
	dependency_graph::io::json json;
	dependency_graph::io::to_json(json, possumwood::App::instance().graph(), selection);

	std::stringstream ss;
	ss << std::setw(4) << json;

	// and put it to the clipboard
	QApplication::clipboard()->setText(ss.str().c_str());
}

dependency_graph::Selection Actions::paste() {
	dependency_graph::Selection selection;

	try {
		// convert the selection to JSON object
		auto json = dependency_graph::io::json::parse(QApplication::clipboard()->text().toStdString());

		// import the clipboard
		selection = dependency_graph::io::from_json(json, possumwood::App::instance().graph(), false);

		// and move all selected nodes by (10, 10)
		for(dependency_graph::Node& n : selection.nodes()) {
			possumwood::NodeData d = n.blindData<possumwood::NodeData>();
			d.setPosition(QPointF(20, 20) + d.position());
			n.setBlindData(d);
		}

	} catch(std::exception& e) {
		// do nothing
		// std::cout << e.what() << std::endl;
	}

	return selection;
}

void Actions::remove(const dependency_graph::Selection& selection) {
	for(auto& e : selection.connections()) {
		auto& n1 = e.from.get().node();
		auto& n2 = e.to.get().node();

		n1.port(e.from.get().index()).disconnect(n2.port(e.to.get().index()));
	}

	for(auto& n : selection.nodes()) {
		auto& graph = possumwood::App::instance().graph();
		auto it = std::find_if(graph.nodes().begin(), graph.nodes().end(), [&](const dependency_graph::Node& i){
			return &i == &n.get();
		});

		assert(it != graph.nodes().end());

		graph.nodes().erase(it);
	}
}

