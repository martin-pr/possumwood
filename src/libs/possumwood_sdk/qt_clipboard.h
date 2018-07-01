#pragma once

#include "actions/clipboard.h"

namespace possumwood {

/// A pure virtual interface for handling clipboard data.
/// A framework-specific implementation provided as a derived class.
class QtClipboard : public Clipboard {
	public:
		virtual void setClipboardContent(const std::string& val) override;
		virtual std::string clipboardContent() const override;

	private:
		static QtClipboard m_instance;
};

}
