#include <memory>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/source_editor.h>

#include <QPainter>
#include <QPushButton>
#include <QMenu>

#include <lualib.h>
#include <luabind/luabind.hpp>
#include <luabind/class_info.hpp>

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
			m_varsButton = new QPushButton("Globals");
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
			if(attr == a_state) {
				if(m_popup)
					m_popup->deleteLater();

				populateVariableList();
			}

			else
				SourceEditor::valueChanged(attr);
		}

		std::string luaType(const luabind::object& o) {
			int type = luabind::type(o);

			switch(type) {
				case LUA_TNONE: return "none";
				case LUA_TNIL: return "nil";
				case LUA_TBOOLEAN: return "bool";
				case LUA_TLIGHTUSERDATA: return "light object";
				case LUA_TNUMBER: return "number";
				case LUA_TSTRING: return "string";
				case LUA_TTABLE: return "table";
				case LUA_TFUNCTION: return "function";
				case LUA_TTHREAD: return "thread";
				case LUA_TUSERDATA:
					if(isClassInstance(o))
						return "object";
					else if(isClass(o))
						return "class";
					else
						return "lua class";
			}

			return "unknown";
		}

		// add one item to the menu
		// TODO: extend to add values as well
		void addItem(QMenu* menu, const luabind::object& o, const std::string& name) {
			menu->addAction((name + "\t" + luaType(o)).c_str());
		}

		// add a submenu to the popup
		QMenu* addMenu(QMenu* menu, const luabind::object& o, const std::string& name) {
			return menu->addMenu((name + "\t" + luaType(o)).c_str());
		}

		void parseClass(QMenu* menu, const luabind::object& o, int recurse) {
			o.push(o.interpreter());
			luabind::detail::object_rep* obj = luabind::detail::get_instance(o.interpreter(), -1);
			if(obj && obj->crep()) {
				obj->crep()->get_table(o.interpreter());
				luabind::object ooo(luabind::from_stack(o.interpreter(), -1));

				if(recurse > 0)
					parseGlobals(menu, ooo, recurse-1);

				lua_settop(o.interpreter(), 0);
			}
		}

		bool isClass(const luabind::object& o) {
			if(o.is_valid()) {
				auto meta = luabind::getmetatable(o);

				if(meta.is_valid() && luabind::type(meta) == LUA_TTABLE) {
					for(luabind::iterator j(meta); j != luabind::iterator(); ++j) {
						auto key = luabind::object_cast_nothrow<std::string>(j.key());
						if(key && *key == "__luabind_classrep")
							return true;
					}
				}
			}

			return false;
		}

		bool isClassInstance(const luabind::object& o) {
			auto meta = luabind::getmetatable(o);

			if(luabind::type(meta) == LUA_TTABLE) {
				for(luabind::iterator j(meta); j != luabind::iterator(); ++j) {
					auto key = luabind::object_cast_nothrow<std::string>(j.key());
					if(key && *key == "__luabind_class")
						return true;
				}
			}

			return false;
		}

		// parses the metadata table
		void parseMeta(QMenu* menu, const luabind::object& o, int recurse = 10) {
			if(o.is_valid()) {
				assert(!isClass(o));

				// adds all members for this instance
				if(isClassInstance(o))
					parseClass(menu, o, recurse-1);

				// just descends recursively
				else {
					auto meta = luabind::getmetatable(o);

					if(meta.is_valid() && luabind::type(meta) == LUA_TTABLE)
						parseGlobals(menu, meta, recurse-1);
				}
			}
		}

		// parse a table and add its content to the menu - first explicitly on globals, then recurse
		void parseGlobals(QMenu* menu, const luabind::object& o, int recurse = 10) {
			assert(o.is_valid());
			assert(luabind::type(o) == LUA_TTABLE);

			// iterate over the items of this table
			for(luabind::iterator j(o), end; j != end; ++j) {
				luabind::object obj = *j;

				if(obj.is_valid()) {
					// try to obtain a key as a string - this might be a number of an array, so treat that appropriately
					auto key = luabind::object_cast_nothrow<std::string>(j.key());
					if(key) {
						// the current object is a table - add a submenu and proceed recursively
						if(luabind::type(obj) == LUA_TTABLE) {
							QMenu* m = addMenu(menu, obj, *key);

							if(recurse > 0)
								parseGlobals(m, obj, recurse - 1);
						}

						// a luabind class - only show it as a single item
						else if(luabind::type(obj) == LUA_TUSERDATA && isClass(obj))
							addItem(menu, obj, *key);

						// a userdata object - either an inbuilt class or a luabind instance
						else if(luabind::type(obj) == LUA_TUSERDATA) {
							if(recurse > 0) {
								QMenu* m = addMenu(menu, obj, *key);
								parseMeta(m, obj, recurse - 1);
							}
						}

						// functions, numbers, strings ... just add them to the menu
						else
							addItem(menu, obj, *key);
					}
				}
			}
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

			// parse the global variables of the state from the output (includes injected vars and modules)
			std::shared_ptr<const possumwood::lua::State> state = values().get(a_state);
			if(state)
				parseGlobals(m_popup, state->globals());
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
		// load all standard Lua libraries
		// luaL_openlibs(*state);

		// luabind class info function - allows introspection of luabind classes
		luabind::bind_class_info(*state);

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
