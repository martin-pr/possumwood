#pragma once

#include <possumwood_sdk/editor.h>

#include <QHBoxLayout>
#include <QPlainTextEdit>

namespace possumwood {

class SourceEditor : public possumwood::Editor {
  public:
	SourceEditor(dependency_graph::InAttr<std::string>& src);
	virtual ~SourceEditor();

  protected:
	QHBoxLayout* buttonsLayout() const;
	QPlainTextEdit* editorWidget() const;

	virtual void valueChanged(const dependency_graph::Attr& attr) override;

  private:
	dependency_graph::InAttr<std::string>* m_src;

	QHBoxLayout* m_buttonsLayout;

	QPlainTextEdit* m_editor;

	bool m_blockedSignals;
};

}  // namespace possumwood
