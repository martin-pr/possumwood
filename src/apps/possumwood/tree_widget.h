#pragma once

#include <vector>
#include <map>

#include <boost/signals2.hpp>

#include <QTreeWidget>

#include <dependency_graph/node.h>

class Adaptor;

class TreeWidget : public QWidget {
	public:
		TreeWidget(QWidget* parent, Adaptor* adaptor);
		virtual ~TreeWidget();

	private:
		void onAddNode(dependency_graph::NodeBase& node);
		void onRemoveNode(dependency_graph::NodeBase& node);

		QTreeWidget* m_tree;

		Adaptor* m_adaptor;
		std::map<dependency_graph::UniqueId, QTreeWidgetItem*> m_items;

		std::vector<boost::signals2::connection> m_signals;
};
