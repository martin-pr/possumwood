#pragma once

#include "metadata.h"

namespace dependency_graph {

/// A register of instantiable node types. The client application needs to register
/// saveable node types to this register, to allow them being loaded from a file.
class MetadataRegister {
  private:
	struct Compare {
		bool operator()(const MetadataHandle& h1, const MetadataHandle& h2) const;
		bool operator()(const std::string& h1, const MetadataHandle& h2) const;
		bool operator()(const MetadataHandle& h1, const std::string& h2) const;
		typedef bool is_transparent;
	};

	std::set<MetadataHandle, Compare> m_handles;

  public:
	static MetadataRegister& singleton();

	void add(const MetadataHandle& handle);
	void remove(const MetadataHandle& handle);

	using const_iterator = std::set<MetadataHandle, Compare>::const_iterator;
	const_iterator begin() const;
	const_iterator end() const;

	const_iterator find(const std::string& nodeName) const;

  private:
	MetadataRegister();
};

}  // namespace dependency_graph
