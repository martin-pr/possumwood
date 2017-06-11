#pragma once

#include "datatypes/filename.h"

#include <possumwood_sdk/properties/property.h>

#include <QLineEdit>
#include <QToolButton>

class filename_ui : public possumwood::properties::property<Filename, filename_ui> {
	public:
		filename_ui();
		virtual ~filename_ui();

		virtual void get(Filename& value) const override;
		virtual void set(const Filename& value) override;

		virtual QWidget* widget() override;

	protected:
		virtual void onFlagsChanged(unsigned flags) override;

	private:
		QWidget* m_widget;
		QLineEdit* m_lineEdit;
		QToolButton* m_browseButton;

		Filename m_value;

		QMetaObject::Connection m_lineEditConnection, m_buttonConnection;
};
