#pragma once

#include <string>

#include <boost/noncopyable.hpp>

namespace possumwood {

/// A pure virtual interface for handling clipboard data.
/// A framework-specific implementation provided as a derived class.
class Clipboard : public boost::noncopyable {
	public:
		virtual ~Clipboard();

		/// Returns an instance of Clipboard. Should be explicitly instantiated as a derived class.
		static Clipboard& instance();

		virtual void setClipboardContent(const std::string& val) = 0;
		virtual std::string clipboardContent() const = 0;

	protected:
		Clipboard();

	private:
};

}
