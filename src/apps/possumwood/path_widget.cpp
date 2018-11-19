#include "path_widget.h"

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

const dependency_graph::UniqueId& PathWidget::Path::operator[](std::size_t index) const {
	assert(index < m_path.size());
	return m_path[index];
}

std::size_t PathWidget::Path::size() const {
	return m_path.size();
}

void PathWidget::Path::pop_back() {
	m_path.pop_back();
}

const dependency_graph::UniqueId& PathWidget::Path::back() const {
	return m_path.back();
}

/////

PathWidget::PathWidget(QWidget* parent) : QWidget(parent) {
	m_layout = new QHBoxLayout();
	m_layout->setContentsMargins(0,0,0,0);
	m_layout->setSpacing(0);

	QToolButton* button = new QToolButton();
	m_layout->addWidget(button);
	button->connect(button, &QToolButton::pressed, [this]() {
		emitChangeNetwork(0);
	});

	QHBoxLayout* mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(0,0,0,0);

	m_back = new QToolButton();
	m_back->setIcon(QIcon(":icons/back.png"));
	m_back->setText("Step back (in/out networks)");
	m_back->setEnabled(false);
	connect(m_back, &QToolButton::clicked, this, &PathWidget::goBack);
	mainLayout->addWidget(m_back);

	m_forward = new QToolButton();
	m_forward->setIcon(QIcon(":icons/forward.png"));
	m_forward->setText("Step forward (in/out networks)");
	m_forward->setEnabled(false);
	connect(m_forward, &QToolButton::clicked, this, &PathWidget::goForward);
	mainLayout->addWidget(m_forward);

	mainLayout->addSpacing(8);

	mainLayout->addLayout(m_layout, 1);
	mainLayout->addStretch(1);
	mainLayout->setSpacing(0);
}

void PathWidget::setPath(const Path& path) {
	if(m_history.empty() || path.m_path != m_history.back().m_path) {
		m_future.clear();
		m_history.push_back(path);

		m_forward->setEnabled(false);
		m_back->setEnabled(true);

		goToPath(path);
	}
}

void PathWidget::goForward() {
	if(!m_future.empty()) {
		m_history.push_back(m_future.back());
		m_future.pop_back();

		goToPath(m_history.back());
		emitChangeNetwork(m_history.back().size()-1);
	}

	m_forward->setEnabled(!m_future.empty());
	m_back->setEnabled(m_history.size() > 1);
}

void PathWidget::goBack() {
	if(m_history.size() > 1) {
		m_future.push_back(m_history.back());
		m_history.pop_back();

		goToPath(m_history.back());
		emitChangeNetwork(m_history.back().size()-1);
	}

	m_forward->setEnabled(!m_future.empty());
	m_back->setEnabled(m_history.size() > 1);
}


const PathWidget::Path& PathWidget::path() const {
	if(!m_history.empty())
		return m_history.back();

	static const Path s_path;
	return s_path;
}

void PathWidget::goToPath(const Path& path) {
	// add buttons for additional items
	while(m_layout->count() < (int)path.m_path.size()) {
		QToolButton* button = new QToolButton();

		std::size_t id = m_layout->count();
		connect(button, &QToolButton::pressed, [this, id]() {
			emitChangeNetwork(id);
		});

		m_layout->addWidget(button);
	}

	// set the visibility for any items that don't need to be shown anymore
	for(int i=0;i<m_layout->count(); ++i)
		m_layout->itemAt(i)->widget()->setVisible(i < (int)path.m_path.size());

	// set the button names
	for(int i=0;i<(int)path.m_path.size(); ++i) {
		QToolButton* button = dynamic_cast<QToolButton*>(m_layout->itemAt(i)->widget());
		assert(button != nullptr);

		if(i == 0)
			button->setText("/");
		else {
			auto it = possumwood::App::instance().graph().nodes().find(path.m_path[i], dependency_graph::Nodes::kRecursive);
			if(it != possumwood::App::instance().graph().nodes().end()) {
				button->setText(it->name().c_str());
				button->setVisible(true);
			}
			else
				button->setVisible(false);
		}
	}
}

void PathWidget::emitChangeNetwork(unsigned id) {
	assert(!m_history.empty());

	Path p = m_history.back();
	while(p.m_path.size() > id+1)
		p.m_path.pop_back();

	emit changeCurrentNetwork(p);
}
