#pragma once

#include <dependency_graph/state.h>

#include <QDialog>

class ErrorDialog : public QDialog {
  public:
	ErrorDialog(const dependency_graph::State& state, QWidget* parent, QString title = "Error...");

  private:
};
