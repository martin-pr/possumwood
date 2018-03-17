#pragma once

#include "nodes.h"
#include "connections.h"

namespace dependency_graph {

class Network : public NodeBase {
	public:
		virtual ~Network();

		bool empty() const;
		void clear();

		Nodes& nodes();
		const Nodes& nodes() const;

		Connections& connections();
		const Connections& connections() const;

		virtual Port& port(size_t index) override;
		virtual const Port& port(size_t index) const override;
		virtual const size_t portCount() const override;

		virtual const Metadata& metadata() const override;
		virtual const Datablock& datablock() const override;

		virtual const State& state() const override;

	protected:
		virtual void computeInput(size_t index) override;
		virtual void computeOutput(size_t index) override;
		virtual Datablock& datablock() override;
		virtual void setDatablock(const Datablock& data) override;

		std::unique_ptr<Node> makeNode(const std::string& name, const Metadata* md);

	private:
		Network(Network* parent);

		Nodes m_nodes;
		Connections m_connections;

	friend class Graph;
	friend class Nodes;
};

}
