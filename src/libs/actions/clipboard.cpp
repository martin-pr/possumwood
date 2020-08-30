#include "clipboard.h"

#include <cassert>

namespace possumwood {

namespace {
Clipboard* s_instance = nullptr;
}

/// Returns an instance of Clipboard. Should be explicitly instantiated as a derived class.
Clipboard& Clipboard::instance() {
	assert(s_instance != nullptr);
	return *s_instance;
}

Clipboard::Clipboard() {
	assert(s_instance == nullptr);
	s_instance = this;
}

Clipboard::~Clipboard() {
	assert(s_instance == this);
	s_instance = nullptr;
}

}  // namespace possumwood
