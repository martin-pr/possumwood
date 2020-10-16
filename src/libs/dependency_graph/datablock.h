#pragma once

#include <memory>
#include <vector>

#include "data.h"
#include "metadata.h"

namespace dependency_graph {

class Node;
class Port;

/// A data storage class used by Node implementation.
/// Each data value is strongly typed, and stored as base class pointer.
class Datablock {
  public:
	Datablock(const MetadataHandle& meta);

	void reset(size_t index);

	const Data& data(size_t index) const;
	void setData(size_t index, const Data& data);
	bool isNull(std::size_t index) const;

	const MetadataHandle& meta() const;

  private:
	std::vector<Data> m_data;
	MetadataHandle m_meta;
};

}  // namespace dependency_graph
