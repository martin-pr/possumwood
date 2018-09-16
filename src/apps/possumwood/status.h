#pragma once

#include <QListWidget>

#include <dependency_graph/node.h>

class Status : public QListWidget {
	Q_OBJECT

	public:
		Status(dependency_graph::NodeBase& node);
		virtual ~Status();

	public slots:
		void update();

	private:
		virtual QSize sizeHint() const override;

		dependency_graph::NodeBase* m_node;

		boost::signals2::connection m_updateConnection;
};
