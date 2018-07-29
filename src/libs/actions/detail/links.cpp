#include "links.h"

#include "tools.h"

namespace possumwood { namespace actions { namespace detail {

void doAddLink(const Link& l) {
	dependency_graph::NodeBase& fromNode = detail::findNode(l.fromNode);
	dependency_graph::NodeBase& toNode = detail::findNode(l.toNode);

	assert(!fromNode.port(l.fromPort).isLinked());

	fromNode.port(l.fromPort).linkTo(toNode.port(l.toPort));

	assert(fromNode.port(l.fromPort).isLinked());
	assert(fromNode.port(l.fromPort).linkedTo().index() == l.toPort);
	assert(fromNode.port(l.fromPort).linkedTo().node().index() == l.toNode);
}

void doRemoveLink(const Link& l) {
	dependency_graph::NodeBase& fromNode = detail::findNode(l.fromNode);
	// dependency_graph::NodeBase& toNode = detail::findNode(l.toNode);

	assert(fromNode.port(l.fromPort).isLinked());
	assert(fromNode.port(l.fromPort).linkedTo().index() == l.toPort);
	assert(fromNode.port(l.fromPort).linkedTo().node().index() == l.toNode);

	fromNode.port(l.fromPort).unlink();

	assert(!fromNode.port(l.fromPort).isLinked());
}

possumwood::UndoStack::Action linkAction(const Link& l) {
	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doAddLink, l),
		std::bind(&doRemoveLink,l)
	);

	return action;
};

possumwood::UndoStack::Action unlinkAction(const dependency_graph::Port& p) {
	Link l{
		p.node().index(), p.index(),
		p.linkedTo().node().index(), p.linkedTo().index()
	};

	possumwood::UndoStack::Action action;

	action.addCommand(
		std::bind(&doRemoveLink,l),
		std::bind(&doAddLink, l)
	);

	return action;
}

} } }
