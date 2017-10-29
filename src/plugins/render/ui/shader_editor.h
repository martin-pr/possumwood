#pragma once

#include <possumwood_sdk/editor.h>

#include <QPlainTextEdit>

namespace possumwood {

class ShaderEditor : public possumwood::Editor {
	public:
		ShaderEditor(dependency_graph::InAttr<std::string>& src);
		virtual ~ShaderEditor();

		virtual QWidget* widget() override;

	protected:
		virtual void valueChanged(const dependency_graph::Attr& attr) override;

	private:
		dependency_graph::InAttr<std::string>* m_src;

		QWidget* m_widget;

		QPlainTextEdit* m_editor;

		bool m_blockedSignals;
};

}
