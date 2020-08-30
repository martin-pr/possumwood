#pragma once

#include <possumwood_sdk/config.h>

#include <QDialog>

class ConfigDialog : public QDialog {
	Q_OBJECT

  public:
	ConfigDialog(QWidget* parent, possumwood::Config& config);

  private:
	std::vector<std::function<void()>> m_acceptValueFunctors;
};
