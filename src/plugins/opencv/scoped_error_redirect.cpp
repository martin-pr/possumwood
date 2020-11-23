#include "scoped_error_redirect.h"

namespace possumwood {
namespace opencv {

ScopedErrorRedirect::ScopedErrorRedirect() {
	m_originalCallback = cv::redirectError(&ScopedErrorRedirect::callback, this, &m_originalUserData);
}

ScopedErrorRedirect::~ScopedErrorRedirect() {
	cv::redirectError(m_originalCallback, m_originalUserData);
}

const dependency_graph::State& ScopedErrorRedirect::state() const {
	return m_state;
}

int ScopedErrorRedirect::callback(int status,
                                  const char* func_name,
                                  const char* err_msg,
                                  const char* file_name,
                                  int line,
                                  void* userdata) {
	ScopedErrorRedirect* ptr = static_cast<ScopedErrorRedirect*>(userdata);
	assert(ptr != nullptr);

	std::stringstream ss;
	ss << "OpenCV error: '" << err_msg << "' in " << file_name << ":" << line << " - " << func_name;

	ptr->m_state.addError(ss.str());

	return 0;
}

}  // namespace opencv
}  // namespace possumwood
