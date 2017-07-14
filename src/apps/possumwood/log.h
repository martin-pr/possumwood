#pragma once

#include <QTreeWidget>

class Log : public QTreeWidget {
	Q_OBJECT

	public:
		Log(QWidget* parent = NULL);

		void addMessage(const QIcon& icon, const QString& message);

	protected:

	private:
};
