#include "searchable_menu.h"

#include <cassert>

#include <QKeyEvent>
#include <QApplication>
#include <QWidgetAction>
#include <QLineEdit>

SearchableMenu::SearchableMenu(const QString& name, QWidget* parent) : QMenu(name, parent), m_menu(NULL) {

}

void SearchableMenu::init(QMenu* menu, const QString& path) {
	QList<QAction*> acts = menu->actions();

	for(auto& act : acts) {
		// assemble item path
		QString name = act->text();
		if(!path.isEmpty())
			name = path + " / " + name;

		// skip the line edit widget
		if(dynamic_cast<QWidgetAction*>(act))
			;

		// recursive init
		else if(act->menu())
			init(act->menu(), name);

		// a triggerable action
		else
			m_actions.insert(std::make_pair(name, act));
	}
}

namespace {

class KeyEventFilter : public QObject {
	public:
		KeyEventFilter(QWidget* target) : QObject(target), m_target(target) {
		}

	protected:

		bool eventFilter(QObject *obj, QEvent *event) {
			bool processed = false;

			// keystroke redirection
			if(event->type() == QEvent::KeyPress) {

				QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

				if(
					// moving in the child menu
					keyEvent->key() != Qt::Key_Down && keyEvent->key() != Qt::Key_Up &&
					// activation of selected item
					keyEvent->key() != Qt::Key_Enter && keyEvent->key() != Qt::Key_Return &&
					// first close this menu, then parent
					keyEvent->key() != Qt::Key_Escape) {

					QApplication::sendEvent(m_target, keyEvent);

					processed = true;
				}
			}

			// standard event processing
			if(!processed)
				return QObject::eventFilter(obj, event);
			return true;
		}

	private:
		QWidget* m_target;
};

}

void SearchableMenu::showEvent(QShowEvent* event) {
	if(actions().length() > 0) {
		// get the widget action (assuming only one is present)
		QWidgetAction* wa = NULL;
		{
			QAction* act = actions()[0];
			wa = dynamic_cast<QWidgetAction*>(act);

			if(wa == NULL) {
				wa = new QWidgetAction(this);
				wa->setDefaultWidget(new QLineEdit());

				insertAction(act, wa);
			}
		}
		assert(wa != NULL);

		// get the line edit from wa (again, assuming only one is present)
		QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(wa->defaultWidget());
		assert(lineEdit != NULL);

		// connect the line edit
		connect(lineEdit, &QLineEdit::textEdited, this, &SearchableMenu::onTextEdited);

		// prepare the line edit and grab the focus to listen to keystrokes
		lineEdit->clear();
		lineEdit->setFocus();

		// initialise
		m_actions.clear();

		init(this, "");

		// make sure menu instance exists
		if(m_menu == NULL) {
			m_menu = new QMenu("", this);

			// and install the event filter
			m_menu->installEventFilter(new KeyEventFilter(lineEdit));
		}
		assert(m_menu != NULL);
		m_menu->hide();
	}

	QMenu::showEvent(event);
}

namespace {

bool fuzzyMatch(const QString& fuzzy, const QString& text) {
	// empty fuzzy string matches any text
	if(fuzzy.isEmpty())
		return true;

	// counter of matched characters
	int counter = 0;

	// iterate over matched string
	for(auto& c : text) {
		// try to match a single character from the fuzzy string
		if(c == fuzzy[counter])
			++counter;

		// if all characters of the fuzzy string were matched, return success
		if(counter == fuzzy.length())
			return true;
	}

	// reached the end of the matched string without matching all fuzzy
	//   characters - failure
	return false;
}

}

void SearchableMenu::onTextEdited(const QString& text) {
	// clear the menu
	assert(m_menu);
	m_menu->hide();
	m_menu->clear();

	// get the line editor
	assert(actions().length() > 0);
	QWidgetAction* wa = dynamic_cast<QWidgetAction*>(actions()[0]);
	assert(wa);

	QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(wa->defaultWidget());
	assert(lineEdit);

	// filter the actions
	for(auto& a : m_actions)
		if(fuzzyMatch(text, a.first)) {
			// positive match - lets make a copy of the action
			QAction* act = m_menu->addAction(a.first);
			connect(act, &QAction::triggered, [a, this]() {
				// triggering the original action
				a.second->triggered();

				// closing the menu itself
				m_menu->close();

				// and closing the parent menu (i.e., SearchableMenu instance)
				close();
			});

			// if there is only one action left in the menu, make it active
			if(m_menu->actions().length() == 1)
				m_menu->setActiveAction(act);
		}

	// display the menu as a popup (non-modal)
	if(!text.isEmpty() && !m_menu->actions().empty()) {
		QPoint pos = lineEdit->mapToGlobal(lineEdit->pos());
		pos.setX(pos.x() + lineEdit->width());

		m_menu->popup(pos);
	}

	// and return the focus to the line editor
	lineEdit->setFocus();
}
