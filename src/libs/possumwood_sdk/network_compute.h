#pragma once

#include <boost/noncopyable.hpp>

#include <dependency_graph/port.h>
#include <dependency_graph/values.h>

namespace possumwood {

/// A simple wrapper over network's compute - delegates computation of external ports to internal ones
class NetworkCompute : public boost::noncopyable {
	public:
		void addInput(const dependency_graph::Port& externalInput, const dependency_graph::NodeBase& inputNode);
		void addOutput(const dependency_graph::Port& externalOutput, const dependency_graph::NodeBase& outputNode);

		void computeOutput(dependency_graph::Values& vals) const;

	private:
};

}
