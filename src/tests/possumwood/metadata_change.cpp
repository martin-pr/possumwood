#include <actions/actions.h>
#include <dependency_graph/graph.h>
#include <possumwood_sdk/app.h>

#include <boost/test/unit_test.hpp>
#include <dependency_graph/attr.inl>
#include <dependency_graph/metadata.inl>
#include <dependency_graph/nodes_iterator.inl>
#include <dependency_graph/port.inl>
#include <dependency_graph/values.inl>

#include "common.h"

using namespace dependency_graph;

namespace {
template <typename T>
bool checkPort(const Port& p, const std::string& name, const Attr::Category& cat) {
	if(p.name() != name)
		return false;

	if(p.category() != cat)
		return false;

	if(p.type() != typeid(T))
		return false;

	return true;
}

struct ConnectionItem {
	dependency_graph::UniqueId fromNode;
	std::size_t fromPort;
	dependency_graph::UniqueId toNode;
	std::size_t toPort;
};

bool checkConnections(const dependency_graph::Network& net, const std::vector<ConnectionItem>& conns) {
	if(conns.size() != net.connections().size()) {
		std::cout << "checkConnections - size not matching - " << conns.size() << " != " << net.connections().size()
		          << std::endl;
		return false;
	}

	auto i1 = net.connections().begin();
	auto i2 = conns.begin();

	while(i1 != net.connections().end()) {
		if(i1->first.node().index() != i2->fromNode || i1->first.index() != i2->fromPort) {
			std::cout << "checkConnections - fromNode / port not matching - " << i1->first.node().index() << "/"
			          << i1->first.index() << " != " << i2->fromNode << "/" << i2->fromPort << std::endl;
			return false;
		}

		if(i1->second.node().index() != i2->toNode || i1->second.index() != i2->toPort) {
			std::cout << "checkConnections - toNode / port not matching - " << i1->second.node().index() << "/"
			          << i1->second.index() << " != " << i2->toNode << "/" << i2->toPort << std::endl;
			return false;
		}

		++i1;
		++i2;
	}

	return true;
}
}  // namespace

BOOST_AUTO_TEST_CASE(meta_single_node) {
	// make the app "singleton"
	possumwood::AppCore app;

	// create node using an Action
	UniqueId id;
	possumwood::actions::createNode(app.graph(), additionNode(), "first", possumwood::NodeData(), id);

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
	possumwood::actions::changeMetadata(node, multiplicationNode());

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
	possumwood::actions::createNode(app.graph(), additionNode(), "first", possumwood::NodeData(), id);

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
	possumwood::actions::changeMetadata(node, intAdditionNode());

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
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 3.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<int>(), 0);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 3.0f);

	// ok, we have an int 0 there, as default value, because the second input was not mapped.
	// lets use an action to set this value to something else, testable
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(node.port(1), 5));
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 8.0f);

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
	BOOST_CHECK_EQUAL(node.port(0).get<float>(), 8.0f);
	BOOST_CHECK_EQUAL(node.port(1).get<int>(), 5);
	BOOST_CHECK_EQUAL(node.port(2).get<float>(), 3.0f);
}

BOOST_AUTO_TEST_CASE(meta_connected_node) {
	// make the app "singleton"
	possumwood::AppCore app;

	// create nodes using an Action
	UniqueId id_front, id_middle, id_back;
	BOOST_REQUIRE_NO_THROW(
	    possumwood::actions::createNode(app.graph(), additionNode(), "front", possumwood::NodeData(), id_front));
	BOOST_REQUIRE_NO_THROW(
	    possumwood::actions::createNode(app.graph(), additionNode(), "middle", possumwood::NodeData(), id_middle));
	BOOST_REQUIRE_NO_THROW(
	    possumwood::actions::createNode(app.graph(), additionNode(), "back", possumwood::NodeData(), id_back));

	// make sure they have been created, and get their pointers
	NodeBase *front, *middle, *back;
	{
		auto it_front = app.graph().nodes().find(id_front, dependency_graph::Nodes::kRecursive);
		BOOST_REQUIRE(it_front != app.graph().nodes().end());
		front = &(*it_front);
		BOOST_CHECK(front->metadata() == additionNode());

		auto it_middle = app.graph().nodes().find(id_middle, dependency_graph::Nodes::kRecursive);
		BOOST_REQUIRE(it_middle != app.graph().nodes().end());
		middle = &(*it_middle);
		BOOST_CHECK(middle->metadata() == additionNode());

		auto it_back = app.graph().nodes().find(id_back, dependency_graph::Nodes::kRecursive);
		BOOST_REQUIRE(it_back != app.graph().nodes().end());
		back = &(*it_back);
		BOOST_CHECK(back->metadata() == additionNode());
	}

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 3u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// make a few connections, and set a few values, all using actions
	BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(front->port(2), middle->port(0)));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(middle->port(2), back->port(0)));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::connect(front->port(2), back->port(1)));

	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(front->port(0), 2.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(front->port(1), 3.0f));
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(middle->port(1), 5.0f));

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 9u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, middle->index(), 0},
	                                              {front->index(), 2, back->index(), 1},
	                                              {middle->index(), 2, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 15.0f);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 10.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	// change the metadata of the middle node
	possumwood::actions::changeMetadata(*middle, multiplicationNode());

	BOOST_CHECK(middle->metadata() == multiplicationNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 10u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 0},
	                                              {middle->index(), 2, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 30.0f);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 25.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	// undo the last action
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	BOOST_CHECK(middle->metadata() == additionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 9u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 1u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 0},
	                                              {middle->index(), 2, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 15.0f);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 10.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	// redo the last action
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	BOOST_CHECK(middle->metadata() == multiplicationNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 10u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 0},
	                                              {middle->index(), 2, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 30.0f);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 25.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	//////////////////////////

	// change the metadata of the middle node to the int addition
	BOOST_REQUIRE_NO_THROW(possumwood::actions::changeMetadata(*middle, intAdditionNode()));

	BOOST_CHECK(middle->metadata() == intAdditionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 11u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// check connections - output is now #0, and float input is #2
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 2},
	                                              {middle->index(), 0, back->index(), 0},
	                                          }));

	// check values on the ports of the new node
	BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 5.0f);
	BOOST_CHECK_EQUAL(middle->port(1).get<int>(), 0);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 5.0f);

	// make the middle one's int input something more interesting
	BOOST_REQUIRE_NO_THROW(possumwood::actions::setValue(middle->port(1), 3));

	BOOST_CHECK_EQUAL(middle->port(1).get<int>(), 3);
	BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 8.0f);

	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 12u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 13.0f);
	BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 8.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	// undo the last two actions, to get back to the original state
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());
	BOOST_REQUIRE_NO_THROW(app.undoStack().undo());

	BOOST_CHECK(middle->metadata() == multiplicationNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 10u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 2u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 0},
	                                              {middle->index(), 2, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 30.0f);
	BOOST_CHECK_EQUAL(middle->port(2).get<float>(), 25.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);

	// redo the last two actions, to get back to the original state
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());
	BOOST_REQUIRE_NO_THROW(app.undoStack().redo());

	BOOST_CHECK(middle->metadata() == intAdditionNode());

	// check the state of the undo stack
	BOOST_CHECK_EQUAL(app.undoStack().undoActionCount(), 12u);
	BOOST_CHECK_EQUAL(app.undoStack().redoActionCount(), 0u);

	// check connections
	BOOST_CHECK(checkConnections(app.graph(), {
	                                              {front->index(), 2, back->index(), 1},
	                                              {front->index(), 2, middle->index(), 2},
	                                              {middle->index(), 0, back->index(), 0},
	                                          }));

	// and test the "pull"
	BOOST_CHECK_EQUAL(back->port(2).get<float>(), 13.0f);
	BOOST_CHECK_EQUAL(middle->port(0).get<float>(), 8.0f);
	BOOST_CHECK_EQUAL(front->port(2).get<float>(), 5.0f);
}
