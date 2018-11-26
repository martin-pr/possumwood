#pragma once

#include <vector>
#include <map>

#include <boost/signals2.hpp>

#include <QTreeWidget>

#include <dependency_graph/node.h>
#include <dependency_graph/selection.h>

class Adaptor;

class TreeWidget : public QWidget {
	Q_OBJECT

	public:
		TreeWidget(QWidget* parent, Adaptor* adaptor);
		virtual ~TreeWidget();

	public slots:
		void onSelectionChanged(dependency_graph::Selection selection);

	private slots:
		void onCurrentNetworkChanged(const dependency_graph::NodeBase& node);

	private:
		void onAddNode(dependency_graph::NodeBase& node);
		void onRemoveNode(dependency_graph::NodeBase& node);


		QTreeWidget* m_tree;

		Adaptor* m_adaptor;
		std::map<dependency_graph::UniqueId, QTreeWidgetItem*> m_items;

		std::vector<boost::signals2::connection> m_signals;
};
