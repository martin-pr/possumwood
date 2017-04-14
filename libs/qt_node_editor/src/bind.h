#pragma once

#include <functional>

#include <QObject>

namespace node_editor {

/// A boost-compatible signal binding object, based on
/// http://uint32t.blogspot.co.uk/2008/11/using-boost-bind-and-boost-function.html .
/// Allows to use boost::bind and std::function (0-argument resulting functor)
///	to be bound as a 0-argument slot.
class BindSignal : public QObject {
		Q_OBJECT

	public:
		BindSignal(QObject* parent, const std::function<void()>& fn);

	public slots:
		void handler();

	private:
		std::function<void()> m_fn;
};

/// a class representing an existing connection, allowing to disconnect it
class BindConnection {
	public:
		BindConnection();
		BindConnection(BindSignal* object);

		/// disconnects everything from the bound object and requests its destruction
		void disconnect();

	protected:

	private:
		BindSignal* m_ptr;
};

const BindConnection qt_bind(QObject* sender, const char* signal, const std::function<void()>& fn, Qt::ConnectionType type = Qt::AutoConnection);

}
