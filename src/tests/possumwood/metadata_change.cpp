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

namespace {
	template<typename T>
	bool checkPort(const Port& p, const std::string& name, const Attr::Category& cat) {
		if(p.name() != name)
			return false;

		if(p.category() != cat)
			return false;

		if(p.type() != unmangledTypeId<T>())
			return false;

		return true;
	}
}

BOOST_AUTO_TEST_CASE(meta_single_node) {
	// make the app "singleton"
	possumwood::AppCore app;

	// create node using an Action
	UniqueId id;
	possumwood::Actions::createNode(app.graph(), additionNode(), "first", possumwood::NodeData(), id);

	// make sure it has been created, and get its pointer
	auto it = app.graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	BOOST_REQUIRE(it != app.graph().nodes().end());
	BOOST_CHECK(it->metadata() == additionNode());
	NodeBase& node = *it;

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test that the attributes are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 0.0f);
	BOOST_REQUIRE_NO_THROW(node.port(0).set(3.0f));
	BOOST_REQUIRE_NO_THROW(node.port(1).set(2.0f));
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 5.0f);

	// change the metadata of the node
	possumwood::Actions::changeMetadata(node, multiplicationNode());

	BOOST_CHECK(it->metadata() == multiplicationNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 2u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test that the attributes are right - there is 1:1 correspondence between original and new metadata
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull" - the values should have been transferred
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 6.0f);

	// undo the last action
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	BOOST_CHECK(it->metadata() == additionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

	// test again that the attrs are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 5.0f);

	// redo the last action
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	BOOST_CHECK(it->metadata() == multiplicationNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 2u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test again that the attrs are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 6.0f);
}

BOOST_AUTO_TEST_CASE(meta_single_differing_nodes) {
	// make the app "singleton"
	possumwood::AppCore app;

	// create node using an Action
	UniqueId id;
	possumwood::Actions::createNode(app.graph(), additionNode(), "first", possumwood::NodeData(), id);

	// make sure it has been created, and get its pointer
	auto it = app.graph().nodes().find(id, dependency_graph::Nodes::kRecursive);
	BOOST_REQUIRE(it != app.graph().nodes().end());
	BOOST_CHECK(it->metadata() == additionNode());
	NodeBase& node = *it;

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test that the attributes are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 0.0f);
	BOOST_REQUIRE_NO_THROW(node.port(0).set(3.0f));
	BOOST_REQUIRE_NO_THROW(node.port(1).set(2.0f));
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 5.0f);

	// change the metadata of the node
	possumwood::Actions::changeMetadata(node, intAdditionNode());

	BOOST_CHECK(it->metadata() == intAdditionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 2u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test that the attributes are right - these should be matched
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "output", Attr::Category::kOutput));
	BOOST_CHECK(checkPort<int>(node.port(1), "input_int", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "input_float", Attr::Category::kInput));

	// and test the "pull" - the values should have been transferred
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 0.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<int>(), 0);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 3.0f);

	// ok, we have an int 0 there, as default value, because the second input was not mapped.
	// lets use an action to set this value to something else, testable
	BOOST_REQUIRE_NO_THROW(possumwood::Actions::setValue(node.port(1), 5));
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 15.0f);

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 3u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// undo the last two actions
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	BOOST_CHECK(it->metadata() == additionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 1u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 2u);

	// test again that the attrs are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "input_1", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(1), "input_2", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "output", Attr::Category::kOutput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<float>(), 2.0f);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 5.0f);

	// redo the last two actions
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	BOOST_CHECK(it->metadata() == intAdditionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 3u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// test again that the attrs are right
	BOOST_REQUIRE_EQUAL(node.portCount(), 3u);
	BOOST_CHECK(checkPort<float>(node.port(0), "output", Attr::Category::kOutput));
	BOOST_CHECK(checkPort<int>(node.port(1), "input_int", Attr::Category::kInput));
	BOOST_CHECK(checkPort<float>(node.port(2), "input_float", Attr::Category::kInput));

	// and test the "pull"
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 15.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<int>(), 5);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 3.0f);
}
