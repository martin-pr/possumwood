#pragma once

#include <sstream>

#include <dependency_graph/state.h>

namespace possumwood {

/// captures standard output and error, and and allows to convert it to a dependency_graph::State instance.
class ScopedOutputRedirect {
  public:
	ScopedOutputRedirect();
	~ScopedOutputRedirect();

	ScopedOutputRedirect(const ScopedOutputRedirect&) = delete;
	ScopedOutputRedirect& operator=(const ScopedOutputRedirect&) = delete;

	/// convert captured output to a state instance - cout to INFO, cerr to ERROR
	dependency_graph::State state() const;

  private:
	std::streambuf* m_origOutBuf;
	std::stringstream m_out;

	std::streambuf* m_origErrBuf;
	std::stringstream m_err;
};

}  // namespace possumwood
