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
dependency_graph::OutAttr<possumwood::lua::State> a_state;

class Editor : public possumwood::SourceEditor {
	public:
		Editor() : SourceEditor(a_src), m_popup(nullptr) {
			m_varsButton = new QPushButton("Globals");
			buttonsLayout()->insertWidget(0, m_varsButton);

			connect(m_varsButton, &QPushButton::pressed, [this]() {
				// lazily populate variable list on first click
				if(m_popup == nullptr)
					populateVariableList();

				// if the populating succeeded, show the menu
				if(m_popup)
					m_popup->popup(
						m_varsButton->mapToGlobal(QPoint(0,-m_popup->sizeHint().height()))
					);
			});
		}

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override {
			// update the menu on each change of the "state" attribute (i.e., evaluation)
			if(attr == a_state && values().get(a_state) != nullptr) {
				if(m_popup) {
					m_popup->deleteLater();
					m_popup = nullptr;
				}
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

		// sorting mechanism for new items
		QAction* findPosition(QMenu* menu, const QString& name) {
			QList<QAction*> actions = menu->actions();

			// find the item
			auto it = std::lower_bound(actions.begin(), actions.end(), name, [](QAction* a, const QString& n) {
				if(a != nullptr)
					return a->text().mid(0, a->text().indexOf('\t')) < n;
				return true;
			});

			// return the value if valid, null otherwise
			if(it != actions.end())
				return *it;
			return nullptr;
		}

		// add one item to the menu
		// TODO: extend to add values as well
		void addItem(QMenu* menu, const luabind::object& o, const std::string& name, std::string instanceText) {
			if(luabind::type(o) == LUA_TFUNCTION)
				instanceText += "()";

			QAction* newAction = new QAction(QString::fromStdString(name + "\t" + luaType(o)), nullptr);
			newAction->setData(QString::fromStdString(instanceText));

			menu->insertAction(findPosition(menu, name.c_str()), newAction);
		}

		// add a submenu to the popup
		QMenu* addMenu(QMenu* menu, const luabind::object& o, const std::string& name) {
			QMenu* newMenu = new QMenu(QString::fromStdString(name + "\t" + luaType(o)));

			menu->insertMenu(findPosition(menu, name.c_str()), newMenu);

			return newMenu;
		}

		void parseClass(QMenu* menu, const luabind::object& o, const std::string& prepend, int recurse) {
			o.push(o.interpreter());
			luabind::detail::object_rep* obj = luabind::detail::get_instance(o.interpreter(), -1);
			if(obj && obj->crep()) {
				obj->crep()->get_table(o.interpreter());
				luabind::object ooo(luabind::from_stack(o.interpreter(), -1));

				if(recurse > 0)
					parseGlobals(menu, ooo, prepend, recurse-1);

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
		void parseMeta(QMenu* menu, const luabind::object& o, const std::string& prepend, int recurse) {
			if(o.is_valid()) {
				assert(!isClass(o));

				// adds all members for this instance
				if(isClassInstance(o))
					parseClass(menu, o, prepend, recurse-1);

				// just descends recursively
				else {
					auto meta = luabind::getmetatable(o);

					if(meta.is_valid() && luabind::type(meta) == LUA_TTABLE)
						parseGlobals(menu, meta, prepend, recurse-1);
				}
			}
		}

		// parse a table and add its content to the menu - first explicitly on globals, then recurse
		void parseGlobals(QMenu* menu, const luabind::object& o, const std::string& prepend = "", int recurse = 10) {
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
								parseGlobals(m, obj, prepend + *key + ".", recurse - 1);
						}

						// a luabind class - only show it as a single item
						else if(luabind::type(obj) == LUA_TUSERDATA && isClass(obj))
							addItem(menu, obj, *key, prepend + *key + ":");

						// a userdata object - either an inbuilt class or a luabind instance
						else if(luabind::type(obj) == LUA_TUSERDATA) {
							if(recurse > 0) {
								QMenu* m = addMenu(menu, obj, *key);
								parseMeta(m, obj, prepend + *key + ":", recurse - 1);
							}
						}

						// functions, numbers, strings ... just add them to the menu
						else
							addItem(menu, obj, *key, prepend + *key);
					}
				}
			}
		}

		void populateVariableList() {
			if(m_popup)
				m_popup->deleteLater();

			m_popup = new QMenu(m_varsButton);

			m_popup->connect(m_popup, &QMenu::triggered, [this](QAction* action) {
				editorWidget()->insertPlainText(action->data().toString());
			});

			// parse the global variables of the state from the output (includes injected vars and modules)
			const possumwood::lua::State& state = values().get(a_state);
			if(state)
				parseGlobals(m_popup, state.globals());
		}

	private:
		QPushButton* m_varsButton;

		QMenu* m_popup;
};

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State result;

	const std::string& src = data.get(a_src);

	// Create a new lua state
	possumwood::lua::State state(data.get(a_context));

	try {
		// load all standard Lua libraries
		luaL_openlibs(state);

		// luabind class info function - allows introspection of luabind classes
		luabind::bind_class_info(state);

		// evaluate our script
		int err = luaL_dostring(state, src.c_str());

		std::string errstr;
		if(err)
			errstr = lua_tostring(state, -1);

		// and return the resulting state
		data.set(a_state, std::move(state));

		if(err)
			throw std::runtime_error(errstr);
	}
	catch(const luabind::error& err) {
		throw std::runtime_error(lua_tostring(err.state(), -1));
	}

	return result;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_src, "source", std::string("variable = 10\n"));
	meta.addAttribute(a_context, "context", possumwood::lua::Context(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_state, "state");

	meta.addInfluence(a_src, a_state);
	meta.addInfluence(a_context, a_state);

	meta.setCompute(&compute);
	meta.setEditor<Editor>();
}

possumwood::NodeImplementation s_impl("lua/script", init);

}
