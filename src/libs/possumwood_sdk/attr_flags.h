#pragma once

namespace possumwood {

/// strongly-typed attribute flags class
enum class AttrFlags {
	kHorizontal = 0, //< default
	kVertical = 1,
};

/// flags combination operator
inline AttrFlags operator | (const AttrFlags& f1, const AttrFlags& f2) {
	return AttrFlags(static_cast<unsigned>(f1) | static_cast<unsigned>(f2));
}

/// flags test
inline bool operator & (const AttrFlags& f1, const AttrFlags& f2) {
	return static_cast<unsigned>(f1) & static_cast<unsigned>(f2);
}

}
