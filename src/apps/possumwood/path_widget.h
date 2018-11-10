#pragma once

#include <vector>

#include <dependency_graph/unique_id.h>

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

namespace possumwood {
	class Index;
};

class PathWidget : public QWidget {
	Q_OBJECT

	public:
		PathWidget(QWidget* parent);

		void setPath(const std::vector<dependency_graph::UniqueId>& ids);
		const std::vector<dependency_graph::UniqueId>& path() const;

	signals:
		void changeCurrentNetwork(dependency_graph::UniqueId id);

	private:
		QHBoxLayout* m_layout;
		std::vector<dependency_graph::UniqueId> m_path;
};
