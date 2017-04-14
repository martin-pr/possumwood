#include "bind.h"

namespace node_editor {

BindSignal::BindSignal(QObject* parent, const std::function<void(void)>& fn) : QObject(parent), m_fn(fn) {
}

void BindSignal::handler() {
	m_fn();
}

///////

void BindConnection::disconnect() {
	if(m_ptr != NULL) {
		m_ptr->disconnect();
		m_ptr->deleteLater();
	}
}

BindConnection::BindConnection() : m_ptr(NULL) {
}

BindConnection::BindConnection(BindSignal* object) : m_ptr(object) {
}

///////

const BindConnection qt_bind(QObject* sender, const char* sign, const std::function<void()>& fn, Qt::ConnectionType type) {
	BindSignal* ptr = new BindSignal(sender, fn);
	QObject::connect(sender, sign, ptr, SLOT(handler()), type);

	return BindConnection(ptr);
}

}
