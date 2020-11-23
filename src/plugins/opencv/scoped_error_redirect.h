#pragma once

#include <opencv2/opencv.hpp>

#include "dependency_graph/state.h"

namespace possumwood {
namespace opencv {

class ScopedErrorRedirect {
  public:
	ScopedErrorRedirect();
	~ScopedErrorRedirect();

	const dependency_graph::State& state() const;

	ScopedErrorRedirect(const ScopedErrorRedirect&) = delete;
	ScopedErrorRedirect& operator=(const ScopedErrorRedirect&) = delete;

  private:
	static int callback(int status,
	                    const char* func_name,
	                    const char* err_msg,
	                    const char* file_name,
	                    int line,
	                    void* userdata);

	cv::ErrorCallback m_originalCallback;
	void* m_originalUserData;

	dependency_graph::State m_state;
};

}  // namespace opencv
}  // namespace possumwood
