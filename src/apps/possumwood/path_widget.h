#pragma once

#include <vector>

#include <QHBoxLayout>
#include <QToolButton>
#include <QWidget>

#include <dependency_graph/node.h>

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

		const dependency_graph::UniqueId& operator[](std::size_t index) const;
		std::size_t size() const;

		void pop_back();
		const dependency_graph::UniqueId& back() const;

	  private:
		std::vector<dependency_graph::UniqueId> m_path;

		friend class PathWidget;
	};

	PathWidget(QWidget* parent);

	void setPath(const Path& path);
	const Path& path() const;

  signals:
	void changeCurrentNetwork(Path path);

  private:
	void goToPath(const Path& path);
	void goForward();
	void goBack();

	void emitChangeNetwork(unsigned id);

	QHBoxLayout* m_layout;

	QToolButton* m_forward;
	QToolButton* m_back;

	std::vector<Path> m_history, m_future;
};
