#pragma once

#include <boost/filesystem/path.hpp>

#include "connections.h"
#include "nodes.h"

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

	// Source is set if this network has been loaded from a file.
	// Changes the way this data gets serialized to a scene file - networks with a source won't get written explicitly.
	void setSource(const boost::filesystem::path& path);
	const boost::filesystem::path& source() const;

  protected:
	Network(const std::string& name, const UniqueId& id, const MetadataHandle& md, Network* parent);

	static const MetadataHandle& defaultMetadata();

  private:
	Nodes m_nodes;
	Connections m_connections;

	boost::filesystem::path m_source;

	friend class Nodes;
	friend class Metadata;
};

}  // namespace dependency_graph
