#pragma once

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#include <dependency_graph/attr.h>
#include <dependency_graph/values.h>

class QWidget;

namespace possumwood {

class Metadata;
class Node;

/// A simple class allowing to edit a Node's values using a custom widget.
class Editor : public boost::noncopyable {
	public:
		Editor();
		virtual ~Editor() = 0;

		/// should be reimplemented to return the QWidget held by this Editor
		virtual QWidget* widget() = 0;

	protected:
		/// callback for changes of values of associated node
		virtual void valueChanged(const dependency_graph::Attr& attr);

		/// returns the values access object for associated node
		dependency_graph::Values& values();
		/// returns the values access object for associated node
		const dependency_graph::Values& values() const;


	private:
		void setNodeReference(dependency_graph::Node& node);

		std::unique_ptr<dependency_graph::Values> m_values;
		std::vector<boost::signals2::connection> m_connections;

		std::set<unsigned> m_runningValueCallbacks;

	friend class Metadata;
};

}
