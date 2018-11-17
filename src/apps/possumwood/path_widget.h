#pragma once

#include <vector>

#include <dependency_graph/node.h>

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

namespace possumwood {
	class Index;
};

class PathWidget : public QWidget {
	Q_OBJECT

	public:
		class Path {
			public:
				Path();
				Path(const dependency_graph::Network& network);

			private:
				std::vector<dependency_graph::UniqueId> m_path;

			friend class PathWidget;
		};

		PathWidget(QWidget* parent);

		void setPath(const Path& path);
		const Path& path() const;

	signals:
		void changeCurrentNetwork(dependency_graph::UniqueId id);

	private:
		QHBoxLayout* m_layout;

		Path m_path;
};
