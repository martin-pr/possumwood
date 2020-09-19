#include "node_menu.h"

#include <actions/actions.h>
#include <actions/node_data.h>
#include <possumwood_sdk/app.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem.hpp>

#include "searchable_menu.h"

namespace {

QAction* makeAction(QString title, std::function<void()> fn, QWidget* parent) {
	QAction* result = new QAction(title, parent);
	QObject::connect(result, &QAction::triggered, [fn](bool) { fn(); });
	return result;
}

}  // namespace

NodeMenu::NodeMenu(Adaptor* a) : m_adaptor(a), m_newNodeMenu(new SearchableMenu("Create\tTab")) {
	// raw ptr for binding
	auto newNodeMenuPtr = m_newNodeMenu.get();
	Adaptor* adaptor = m_adaptor;

	{
		// top-level action to open the menu
		QAction* newNodeAction = new QAction("Create node", m_adaptor->graphWidget());
		newNodeAction->setShortcut(Qt::Key_Tab);
		newNodeAction->setShortcutContext(Qt::WidgetShortcut);
		m_adaptor->graphWidget()->addAction(newNodeAction);

		QObject::connect(newNodeAction, &QAction::triggered, [adaptor, newNodeMenuPtr]() {
			if(adaptor->graphWidget()->underMouse()) {
				newNodeMenuPtr->move(QCursor::pos());
				newNodeMenuPtr->popup(QCursor::pos());
			}
		});
	}
}

void NodeMenu::addItem(const std::string& name, std::function<void()> action) {
	std::string itemName = name;

	auto it = itemName.rfind('/');
	if(it != std::string::npos)
		itemName = itemName.substr(it + 1);

	QAction* addNode = makeAction(itemName.c_str(), action, m_adaptor->graphWidget());
	addNode->setIcon(QIcon(":icons/add-node.png"));

	m_items.insert(std::make_pair(name, addNode));
}

void NodeMenu::addFromNodeRegister(const dependency_graph::MetadataRegister& r) {
	// raw pointers for binding
	auto newNodeMenuPtr = m_newNodeMenu.get();
	Adaptor* adaptor = m_adaptor;

	std::map<std::string, QAction*> items;
	for(auto& m : r) {
		std::string itemName = m.metadata().type();

		auto it = m.metadata().type().rfind('/');
		if(it != std::string::npos)
			itemName = m.metadata().type().substr(it + 1);

		addItem(m.metadata().type(), [&m, itemName, newNodeMenuPtr, adaptor]() {
			auto qp = adaptor->graphWidget()->mapToScene(adaptor->graphWidget()->mapFromGlobal(newNodeMenuPtr->pos()));
			const possumwood::NodeData::Point p{(float)qp.x(), (float)qp.y()};

			possumwood::actions::createNode(adaptor->currentNetwork(), m, itemName, p);
		});
	}
}

void NodeMenu::addFromDirectory(const boost::filesystem::path& startPath) {
	// raw pointers for binding
	auto newNodeMenuPtr = m_newNodeMenu.get();
	Adaptor* adaptor = m_adaptor;

	// scan the directory recursively
	boost::filesystem::recursive_directory_iterator it(startPath), end;

	while(it != end) {
		if(boost::filesystem::is_regular_file(it->status())) {
			const boost::filesystem::path path = boost::filesystem::relative(*it, startPath);

			addItem((path.parent_path() / path.stem()).string(), [path, newNodeMenuPtr, adaptor]() {
				auto qp =
				    adaptor->graphWidget()->mapToScene(adaptor->graphWidget()->mapFromGlobal(newNodeMenuPtr->pos()));
				const possumwood::NodeData::Point p{(float)qp.x(), (float)qp.y()};

				dependency_graph::Selection selection;

				dependency_graph::State state = possumwood::actions::importNetwork(
				    adaptor->currentNetwork(), selection, possumwood::Filepath::fromString("$NODES/" + path.string()),
				    path.stem().string(), dependency_graph::Data(possumwood::NodeData(p)));
			});
		}

		++it;
	}
}

std::unique_ptr<QMenu> NodeMenu::build() {
	auto newNodeMenuPtr = m_newNodeMenu.get();  // raw ptr for binding

	// create all "folders"
	std::map<std::string, QMenu*> groups;
	for(auto& m : m_items) {
		std::vector<std::string> pieces;
		boost::split(pieces, m.first, boost::algorithm::is_any_of("/"));
		assert(pieces.size() >= 1);

		for(unsigned a = 1; a < pieces.size(); ++a) {
			const std::string current = boost::join(std::make_pair(pieces.begin(), pieces.begin() + a), "/");

			if(groups.find(current) == groups.end()) {
				QMenu* menu = new QMenu(pieces[a - 1].c_str());

				groups.insert(std::make_pair(current, menu));
			}
		}
	}

	// then, assemble the menu
	for(auto& m : groups) {
		auto it = m.first.rfind('/');
		if(it != std::string::npos) {
			assert(it > 0);
			std::string parentName = m.first.substr(0, it);

			auto menu = groups.find(parentName);
			assert(menu != groups.end());
			menu->second->addMenu(m.second);
		}
	}

	for(auto& i : m_items) {
		auto it = i.first.rfind('/');
		if(it != std::string::npos) {
			std::string parentName = i.first.substr(0, it);

			auto menu = groups.find(parentName);
			assert(menu != groups.end());
			menu->second->addAction(i.second);
		}
	}

	// finally, populate the newNodeMenu
	for(auto& m : groups)
		if(m.first.find('/') == std::string::npos)
			newNodeMenuPtr->addMenu(m.second);
	for(auto& i : m_items)
		if(i.first.find('/') == std::string::npos)
			newNodeMenuPtr->addAction(i.second);

	// clean up any state data - they are no longer needed, and they can't be reused
	m_items.clear();

	return std::move(m_newNodeMenu);
}
