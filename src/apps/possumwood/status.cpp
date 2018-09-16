#include "status.h"

#include <dependency_graph/graph.h>

#include <QIcon>
#include <QLabel>
#include <QResizeEvent>

Status::Status(dependency_graph::NodeBase& node) : m_node(&node) {
	// setWordWrap(true);
	// setTextElideMode(Qt::ElideLeft);

	setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
	setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setFrameStyle(QFrame::NoFrame);

	update();

	m_updateConnection = node.graph().onStateChanged([this](const dependency_graph::NodeBase& n) {
		if(&n == m_node)
			update();
	});
}

Status::~Status() {
	m_updateConnection.disconnect();
}

void Status::update() {
	clear();

	if(m_node->state().size() == 0) {
		QListWidgetItem* item = new QListWidgetItem();

		item->setIcon(QIcon::fromTheme("dialog-information"));

		item->setText("(no errors)");
		item->setToolTip("(no errors)");

		addItem(item);
	}

	else {
		for(auto& i : m_node->state()) {
			QListWidgetItem* item = new QListWidgetItem();

			switch(i.first) {
				case dependency_graph::State::kInfo:
					item->setIcon(QIcon::fromTheme("dialog-information"));
					break;
				case dependency_graph::State::kWarning:
					item->setIcon(QIcon::fromTheme("dialog-warning"));
					break;
				case dependency_graph::State::kError:
					item->setIcon(QIcon::fromTheme("dialog-error"));
					break;
			}

			item->setText(i.second.c_str());
			item->setToolTip(i.second.c_str());

			addItem(item);
		}
	}
}

QSize Status::sizeHint() const {
	QFontMetrics fm(font());

	QSize s = QListWidget::sizeHint();
	s.setHeight(fm.lineSpacing() + 2*spacing());

	return s;
}

