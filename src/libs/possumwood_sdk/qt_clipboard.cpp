#include "qt_clipboard.h"

#include <QApplication>
#include <QClipboard>

namespace possumwood {

void QtClipboard::setClipboardContent(const std::string& val) {
	QApplication::clipboard()->setText(val.c_str());
}

std::string QtClipboard::clipboardContent() const {
	return QApplication::clipboard()->text().toStdString();
}

QtClipboard QtClipboard::m_instance;

}  // namespace possumwood
