#include "path_widget.h"

#include <QToolButton>
#include <QAction>

#include <possumwood_sdk/app.h>

PathWidget::Path::Path() {
}

PathWidget::Path::Path(const dependency_graph::Network& network) {
	m_path.push_back(network.index());

	const dependency_graph::Network* net = &network;
	while(net->hasParentNetwork()) {
		net = &net->network();
		m_path.insert(m_path.begin(), net->index());
	}
}


PathWidget::PathWidget(QWidget* parent) : QWidget(parent) {
	m_layout = new QHBoxLayout();
	m_layout->setContentsMargins(0,0,0,0);
	m_layout->setSpacing(0);

	QToolButton* button = new QToolButton();
	m_layout->addWidget(button);
	button->connect(button, &QToolButton::pressed, [this]() {
		emit changeCurrentNetwork(m_path.m_path[0]);
	});

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0,0,0,0);
	mainLayout->addLayout(m_layout);
	mainLayout->addStretch(1);
	mainLayout->setSpacing(0);
}

void PathWidget::setPath(const Path& path) {
	m_path = path;

	// add buttons for additional items
	while(m_layout->count() < (int)m_path.m_path.size()) {
		QToolButton* button = new QToolButton();

		std::size_t id = m_layout->count();
		connect(button, &QToolButton::pressed, [this, id]() {
			emit changeCurrentNetwork(m_path.m_path[id]);
		});

		m_layout->addWidget(button);
	}

	// set the visibility for any items that don't need to be shown anymore
	for(int i=0;i<m_layout->count(); ++i)
		m_layout->itemAt(i)->widget()->setVisible(i < (int)m_path.m_path.size());

	// set the button names
	for(int i=0;i<(int)m_path.m_path.size(); ++i) {
		std::string name = "/";

		auto it = possumwood::App::instance().graph().nodes().find(m_path.m_path[i], dependency_graph::Nodes::kRecursive);
		if(it != possumwood::App::instance().graph().nodes().end())
			name = it->name();

		QToolButton* button = dynamic_cast<QToolButton*>(m_layout->itemAt(i)->widget());
		assert(button != nullptr);
		button->setText(name.c_str());
	}
}

const PathWidget::Path& PathWidget::path() const {
	return m_path;
}
