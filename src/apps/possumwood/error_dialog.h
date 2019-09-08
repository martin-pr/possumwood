#pragma once

#include <QDialog>

#include <dependency_graph/state.h>

class ErrorDialog : public QDialog {
	public:
		ErrorDialog(const dependency_graph::State& state, QWidget* parent);

	private:
};
