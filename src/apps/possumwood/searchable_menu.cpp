#include "searchable_menu.h"

#include <cassert>

#include "search_action.h"

SearchableMenu::SearchableMenu(QWidget* parent) : QMenu(parent) {

}

SearchableMenu::SearchableMenu(const QString& name, QWidget* parent) : QMenu(name, parent) {

}

void SearchableMenu::showEvent(QShowEvent* event) {
	if(actions().length() > 0) {
		SearchAction* sa = NULL;
		{
			QAction* act = actions()[0];
			sa = dynamic_cast<SearchAction*>(act);

			if(sa == NULL) {
				sa = new SearchAction(this);
				insertAction(act, sa);
			}
		}
		assert(sa != NULL);

		sa->lineEdit()->clear();
		sa->lineEdit()->setFocus();

		sa->init(this);
	}

	QMenu::showEvent(event);
}

void SearchableMenu::hideEvent(QHideEvent* event) {
	QMenu::hideEvent(event);
}
