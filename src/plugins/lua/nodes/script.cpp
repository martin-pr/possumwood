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

		std::string luaType(int type) {
			switch(type) {
				case LUA_TNONE: return "LUA_TNONE";

				case LUA_TNIL: return "LUA_TNIL";
				case LUA_TBOOLEAN: return "LUA_TBOOLEAN";
				case LUA_TLIGHTUSERDATA: return "LUA_TLIGHTUSERDATA";
				case LUA_TNUMBER: return "LUA_TNUMBER";
				case LUA_TSTRING: return "LUA_TSTRING";
				case LUA_TTABLE: return "LUA_TTABLE";
				case LUA_TFUNCTION: return "LUA_TFUNCTION";
				case LUA_TUSERDATA: return "LUA_TUSERDATA";
				case LUA_TTHREAD: return "LUA_TTHREAD";
			}

			return "unknown";
		}

		void printItem(const luabind::object& o, const std::string& name, std::size_t offset = 0, char prefix = '*') {
			for(std::size_t i=0;i<offset;++i)
				std::cout << "  ";

			std::cout << " " << prefix << " " << name << "  " << luaType(luabind::type(o)) << std::endl;
		}

		void printObject(const luabind::object& o, std::size_t offset = 0, char prefix = '*', bool recurse = false) {
			if(o.is_valid() && luabind::type(o) == LUA_TTABLE) {
				for(luabind::iterator j(o); j != luabind::iterator(); ++j) {
					std::stringstream ss;
					ss << j.key();

					printItem(*j, ss.str(), offset, prefix);

					if(recurse)
						printObject(o, offset+1, prefix, true);
				}
			}
		}

		void printMeta(const luabind::object& o, std::size_t offset = 0) {
			if(o.is_valid()) {
				auto meta = luabind::getmetatable(o);
				if(meta.is_valid() && luabind::type(meta) == LUA_TTABLE)
					printObject(meta, offset, 'm', false);

				if(meta["__luabind_classrep"].operator luabind::object().is_valid()) {
					// std::cout << "lua class" << std::endl;

					// auto ci = luabind::get_class_info(o);
				}
			}
		}

		void printGlobals(const luabind::object& o, std::string name = "(root)", std::size_t offset = 0) {
			if(o.is_valid()) {
				printItem(o, name, offset, '-');

				if(luabind::type(o) == LUA_TUSERDATA) {
					printMeta(o, offset+1);
				}

				if(luabind::type(o) == LUA_TTABLE) {
					// printMeta(o, offset+1);

					for(luabind::iterator j(o); j != luabind::iterator(); ++j)
						printGlobals(*j, luabind::object_cast<std::string>(j.key()), offset+1);
				}
			}

			// std::cout << std::endl;
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

			// make a dummy state
			possumwood::lua::State state(values().get(a_context));

			printGlobals(state.globals());

			// variables
			unsigned ctr = 0;
			for(auto& s : values().get(a_context).variables()) {
				m_popup->addItem(s.name() + "\t" + s.str());
				++ctr;
			}

			std::cout << std::endl;
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
	meta.addAttribute(a_context, "context", possumwood::lua::Context(), possumwood::Metadata::kVertical);
	meta.addAttribute(a_state, "state");

	meta.addInfluence(a_src, a_state);
	meta.addInfluence(a_context, a_state);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("lua/script", init);

}
