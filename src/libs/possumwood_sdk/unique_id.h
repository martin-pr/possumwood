#pragma once

#include <cstddef>

namespace possumwood {

/// A simple unique ID class to replace raw pointers in the Adaptor class.
/// Allows to re-create links between UI and the data model in an undo queue,
/// keeping the links intact. Its creation is only via a default constructor,
/// which creates a new unique ID. This can be copied, assigned, and used in
/// sorted indexes, but the same ID will not be generated using the default
/// constructor again.
class UniqueId {
	public:
		UniqueId();

		bool operator == (const UniqueId& id) const;
		bool operator != (const UniqueId& id) const;

		bool operator < (const UniqueId& id) const;

	private:
		std::size_t m_id;
};

}
