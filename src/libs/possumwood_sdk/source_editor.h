#pragma once

#include <possumwood_sdk/editor.h>

#include <QPlainTextEdit>

namespace possumwood {

class SourceEditor : public possumwood::Editor {
	public:
		SourceEditor(dependency_graph::InAttr<std::string>& src);
		virtual ~SourceEditor();

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
