#include "config_dialog.h"

#include <map>
#include <cctype>

#include <boost/algorithm/string/replace.hpp>

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QDialogButtonBox>

#include <possumwood_sdk/config.inl>

namespace {
	std::string toLabel(const std::string& text) {
		std::string result = text;
		boost::replace_all(result, "_", " ");

		bool upper = true;
		for(auto& c : result)
			if(c == ' ')
				upper = true;
			else if(upper) {
				c = std::toupper(c);
				upper = false;
			}

		return result;
	}
}

ConfigDialog::ConfigDialog(QWidget* parent, possumwood::Config& config) {
	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	std::map<std::string, QGroupBox*> groups;

	// for each item in the config
	for(possumwood::Config::Item& i : config) {
		// check if theres a group box to work with, and create a new one if not
		auto grpIt = groups.find(i.group());
		if(grpIt == groups.end()) {
			QGroupBox* grp = new QGroupBox(toLabel(i.group()).c_str());
			grp->setLayout(new QFormLayout());

			grpIt = groups.insert(std::make_pair(i.group(), grp)).first;

			mainLayout->addWidget(grp);
		}

		// get its layout
		QFormLayout* layout = qobject_cast<QFormLayout*>(grpIt->second->layout());
		assert(layout != nullptr);

		// create an appropriate widget (could be generalized), and a functor
		//   to transfer the value back to the config object
		if(i.is<float>()) {
			QDoubleSpinBox* spinbox = new QDoubleSpinBox();
			spinbox->setValue(i.as<float>());
			spinbox->setToolTip(i.description().c_str());

			layout->addRow(toLabel(i.name()).c_str(), spinbox);

			std::string name = i.name();
			m_acceptValueFunctors.push_back([spinbox, &config, name]() {
				config[name] = (float)spinbox->value();
			});
		}
		else if(i.is<int>()) {
			QSpinBox* spinbox = new QSpinBox();
			spinbox->setValue(i.as<int>());
			spinbox->setToolTip(i.description().c_str());

			layout->addRow(toLabel(i.name()).c_str(), spinbox);

			std::string name = i.name();
			m_acceptValueFunctors.push_back([spinbox, &config, name]() {
				config[name] = (int)spinbox->value();
			});
		}
		else if(i.is<std::string>()) {
			QLineEdit* edit = new QLineEdit();
			edit->setText(i.as<std::string>().c_str());
			edit->setToolTip(i.description().c_str());

			layout->addRow(toLabel(i.name()).c_str(), edit);

			std::string name = i.name();
			m_acceptValueFunctors.push_back([edit, &config, name]() {
				config[name] = edit->text().toStdString();
			});
		}
		else
			assert(false);
	}

	// and add dialog buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	mainLayout->addWidget(buttons);

	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// on accept, call all accepted functors
	connect(this, &QDialog::accepted, [this]() {
		for(auto& f : m_acceptValueFunctors)
			f();
	});
}
