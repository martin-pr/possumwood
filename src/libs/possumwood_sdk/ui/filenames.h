#pragma once

#include <possumwood_sdk/properties/property.h>

#include <QTextEdit>
#include <QToolButton>

#include "datatypes/filenames.h"

class filenames_ui : public possumwood::properties::property<possumwood::Filenames, filenames_ui> {
  public:
	filenames_ui();
	virtual ~filenames_ui();

	virtual void get(possumwood::Filenames& value) const override;
	virtual void set(const possumwood::Filenames& value) override;

	virtual QWidget* widget() override;

  protected:
	virtual void onFlagsChanged(unsigned flags) override;

  private:
	QWidget* m_widget;
	QTextEdit* m_textEdit;
	QToolButton* m_browseButton;

	possumwood::Filenames m_value;

	QMetaObject::Connection m_textEditConnection, m_buttonConnection;
};
