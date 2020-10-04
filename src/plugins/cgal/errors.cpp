#include "errors.h"

#include <iostream>

namespace possumwood {

ScopedOutputRedirect::ScopedOutputRedirect() {
	m_origOutBuf = std::cout.rdbuf(m_out.rdbuf());
	m_origErrBuf = std::cerr.rdbuf(m_err.rdbuf());
}

ScopedOutputRedirect::~ScopedOutputRedirect() {
	std::cout.rdbuf(m_origOutBuf);
	std::cerr.rdbuf(m_origErrBuf);
}

dependency_graph::State ScopedOutputRedirect::state() const {
	dependency_graph::State result;

	if(!m_out.str().empty())
		result.addInfo(m_out.str());

	if(!m_err.str().empty())
		result.addError(m_err.str());

	return result;
}

}  // namespace possumwood
