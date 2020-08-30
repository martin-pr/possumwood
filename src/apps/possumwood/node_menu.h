#pragma once

#include <dependency_graph/metadata_register.h>

#include <QMenu>
#include <boost/filesystem/path.hpp>
#include <functional>
#include <memory>

#include "adaptor.h"

class NodeMenu {
  public:
	NodeMenu(Adaptor* adaptor);

	void addFromNodeRegister(const dependency_graph::MetadataRegister& r);
	void addFromDirectory(const boost::filesystem::path& path);

	// builds the menu out of the information provided.
	// Using a raw pointer argument to match Qt signature - the lifetime of the
	// menu is tied to the lifetime of the Adaptor instance.
	std::unique_ptr<QMenu> build();

  protected:
	void addItem(const std::string& name, std::function<void()> action);

  private:
	Adaptor* m_adaptor;
	std::unique_ptr<QMenu> m_newNodeMenu;

	std::map<std::string, QAction*> m_items;
};
