#include "search_action.h"

#include <cassert>

#include <QMenu>
#include <QKeyEvent>
#include <QApplication>

SearchAction::SearchAction(QWidget* parent) : QWidgetAction(parent), m_lineEdit(NULL),
	m_menu(NULL) {
}

SearchAction::~SearchAction() {
}

QWidget* SearchAction::createWidget(QWidget* parent) {
	m_lineEdit = new QLineEdit(parent);

	m_lineEdit->setPlaceholderText("Type to search...");
	connect(m_lineEdit, &QLineEdit::textEdited, this, &SearchAction::onTextEdited);

	return m_lineEdit;
}

QLineEdit* SearchAction::lineEdit() {
	return m_lineEdit;
}

void SearchAction::init(QMenu* menu, const QString& path) {
	QList<QAction*> acts = menu->actions();

	// root action
	if(path.isEmpty())
		m_actions.clear();

	for(auto& act : acts) {
		QString name = act->text();
		if(!path.isEmpty())
			name = path + " / " + name;

		// skip itself
		if(act == this)
			;

		// recursive init
		else if(act->menu()) {
			if(!path.isEmpty())
				init(act->menu(), name);
			else
				init(act->menu(), act->text());
		}

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

bool fuzzyMatch(const QString& fuzzy, const QString& text) {
	if(fuzzy.isEmpty())
		return true;

	int counter = 0;

	for(auto& c : text) {
		if(c == fuzzy[counter])
			++counter;

		if(counter == fuzzy.length())
			return true;
	}

	return false;
}

}

void SearchAction::onTextEdited(const QString& text) {
	if(m_menu == NULL) {
		m_menu = new QMenu("", parentWidget());
		m_menu->installEventFilter(new KeyEventFilter(m_lineEdit));
	}

	m_menu->hide();

	///

	m_menu->clear();

	for(auto& a : m_actions)
		if(fuzzyMatch(text, a.first)) {
			QAction* act = m_menu->addAction(a.first);
			connect(act, &QAction::triggered, [a, this]() {
				a.second->triggered();

				m_menu->close();

				QWidget* par = parentWidget();
				assert(dynamic_cast<QMenu*>(par) != NULL);

				par->close();
			});

			if(m_menu->actions().length() == 1)
				m_menu->setActiveAction(act);
		}

	QPoint pos = m_lineEdit->mapToGlobal(m_lineEdit->pos());
	pos.setX(pos.x() + m_lineEdit->width());

	if(!text.isEmpty() && !m_menu->actions().empty())
		m_menu->popup(pos);
	m_lineEdit->setFocus();
}
