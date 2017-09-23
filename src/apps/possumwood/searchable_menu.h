#pragma once

#include <QMenu>

/// A simple searchable menu, based on a standard menu implementation.
/// On show, it indexes its items, and creates a textbox bar that allows to fuzzily search by name
/// for one of this menu's or submenu's actions.
class SearchableMenu : public QMenu {
	Q_OBJECT

	public:
		SearchableMenu(QWidget* parent = NULL);
		SearchableMenu(const QString& name, QWidget* parent = NULL);

	protected:
		void showEvent(QShowEvent* event) override;
		void hideEvent(QHideEvent* event) override;

	private:
};
