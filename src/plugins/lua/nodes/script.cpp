#include <memory>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/source_editor.h>

#include <QPainter>
#include <QPushButton>
#include <QMenu>

#include <luabind/luabind.hpp>

#include "datatypes/state.h"
#include "datatypes/context.h"

namespace {

dependency_graph::InAttr<std::string> a_src;
dependency_graph::InAttr<possumwood::lua::Context> a_context;
dependency_graph::OutAttr<std::shared_ptr<const possumwood::lua::State>> a_state;

class Popup : public QMenu {
	public:
		Popup(QWidget* parent) : QMenu(parent) {
		}

		void addItem(const std::string& name) {
			addAction(name.c_str());
		}

	private:
};


class Editor : public possumwood::SourceEditor {
	public:
		Editor() : SourceEditor(a_src), m_popup(nullptr) {
			m_varsButton = new QPushButton("Variables");
			buttonsLayout()->insertWidget(0, m_varsButton);

			widget()->connect(m_varsButton, &QPushButton::pressed, [this]() {
				if(m_popup)
					m_popup->popup(
						m_varsButton->mapToGlobal(QPoint(0,-m_popup->sizeHint().height()))
					);
			});
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			if(attr == a_context) {
				if(m_popup)
					m_popup->deleteLater();

				populateVariableList();
			}

			else
				SourceEditor::valueChanged(attr);
		}

		void populateVariableList() {
			if(m_popup)
				m_popup->deleteLater();

			m_popup = new Popup(m_varsButton);

			m_popup->connect(m_popup, &Popup::triggered, [this](QAction* action) {
				QString text = action->text();
				while(!text.isEmpty() && !QChar(text[0]).isLetterOrNumber())
					text = text.mid(1, text.length()-1);
				if(text.indexOf('\t') >= 0)
					text = text.mid(0, text.indexOf('\t'));

				editorWidget()->insertPlainText(text);
			});

			// variables
			unsigned ctr = 0;
			for(auto& s : values().get(a_context).variables()) {
				m_popup->addItem(s.name() + "\t" + s.str());
				++ctr;
			}
		}

	private:
		QPushButton* m_varsButton;

		Popup* m_popup;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	// Create a new lua state
	std::shared_ptr<possumwood::lua::State> state(new possumwood::lua::State(data.get(a_context)));

	try {
		// evaluate our script
		int err = luaL_dostring(*state, src.c_str());
		if(err)
			throw std::runtime_error(lua_tostring(*state, -1));

		// and return the resulting state
		data.set(a_state, std::shared_ptr<const possumwood::lua::State>(state));
	}
	catch(const luabind::error& err) {
		throw std::runtime_error(lua_tostring(err.state(), -1));
	}

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("variable = 10\n"));
	meta.addAttribute(a_context, "context");
	meta.addAttribute(a_state, "state");

	meta.addInfluence(a_src, a_state);
	meta.addInfluence(a_context, a_state);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("lua/script", init);

}
