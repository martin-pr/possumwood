#pragma once

#include <functional>

#include <boost/noncopyable.hpp>

#include "attr.h"

class Datablock;
class Metadata;

class Node {
	public:
		Datablock& data();
		const Datablock& data() const;

		class Port : public boost::noncopyable {
			public:
				const std::string& name() const;
				const Attr::Category category() const;

				/// forces the port dirty, even when no node inputs are
				void markDirty();

				/// returns true if given port is dirty
				bool isDirty() const;

			private:
				Port(const std::string& name);

				friend class Node;
		};

		Port& port(const std::string& name);
		const Port& port(const std::string& name) const;

	protected:
		Node(const std::string& name, const Metadata& def);
};
