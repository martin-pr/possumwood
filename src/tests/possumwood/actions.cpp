#include <boost/test/unit_test.hpp>

#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/graph.h>
#include <dependency_graph/port.inl>
#include <dependency_graph/values.inl>

#include <possumwood_sdk/app.h>
#include <possumwood_sdk/actions.h>

#include "common.h"

using namespace dependency_graph;

namespace dependency_graph {

/// a local implementation of the output operator, to make sure tests build
static std::ostream& operator << (std::ostream& out, const dependency_graph::MetadataHandle& h) {
	return out;
}

}

BOOST_AUTO_TEST_CASE(actions_single_node) {
	// make the app "singleton"
	possumwood::AppCore app;

	// initial state
	BOOST_CHECK(app.graph().nodes().empty());
	BOOST_CHECK(app.graph().connections().empty());

	BOOST_CHECK(app.undoStack().empty());

	// now make a single node using an Action
	BOOST_REQUIRE_NO_THROW(
		possumwood::Actions::createNode(app.graph(), additionNode(), "add_1", possumwood::NodeData())
	);

	// check the state of the graph
	BOOST_REQUIRE_EQUAL(app.graph().nodes().size(), 1u);
	BOOST_CHECK(app.graph().connections().empty());
	{
		dependency_graph::NodeBase& node = *app.graph().nodes().begin();
		BOOST_CHECK_EQUAL(node.metadata(), additionNode());
		BOOST_CHECK_EQUAL(node.name(), "add_1");
	}

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// undo it
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	// check the state of the graph
	BOOST_CHECK(app.graph().nodes().empty());
	BOOST_CHECK(app.graph().connections().empty());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 0u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

	// redo it
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	// check the state of the graph
	BOOST_REQUIRE_EQUAL(app.graph().nodes().size(), 1u);
	BOOST_CHECK(app.graph().connections().empty());
	{
		dependency_graph::NodeBase& node = *app.graph().nodes().begin();
		BOOST_CHECK_EQUAL(node.metadata(), additionNode());
		BOOST_CHECK_EQUAL(node.name(), "add_1");
	}

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	/////////

	// remove the node
	BOOST_REQUIRE_NO_THROW(
		possumwood::Actions::removeNode(*app.graph().nodes().begin());
	);

	// check the state of the graph
	BOOST_CHECK(app.graph().nodes().empty());
	BOOST_CHECK(app.graph().connections().empty());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 2u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// undo it
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	// check the state of the graph
	BOOST_REQUIRE_EQUAL(app.graph().nodes().size(), 1u);
	BOOST_CHECK(app.graph().connections().empty());
	{
		dependency_graph::NodeBase& node = *app.graph().nodes().begin();
		BOOST_CHECK_EQUAL(node.metadata(), additionNode());
		BOOST_CHECK_EQUAL(node.name(), "add_1");
	}

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

	// redo it
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	// check the state of the graph
	BOOST_CHECK(app.graph().nodes().empty());
	BOOST_CHECK(app.graph().connections().empty());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 2u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);
}

/// TODO: connections and evaluation tests
