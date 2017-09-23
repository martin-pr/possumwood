#pragma once

#include <QWidgetAction>
#include <QLineEdit>

class QMenu;

class SearchAction : public QWidgetAction {
	Q_OBJECT

	public:
		SearchAction(QWidget* parent);
		virtual ~SearchAction();

		QLineEdit* lineEdit();

		void init(QMenu* menu, const QString& path = "");

	protected:
		virtual QWidget* createWidget(QWidget* parent) override;

	private slots:
		void onTextEdited(const QString& text);

	private:
		QLineEdit* m_lineEdit;
		QMenu* m_menu;

		std::map<QString, QAction*> m_actions;
};
