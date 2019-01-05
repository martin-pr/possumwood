#pragma once

#include <ImathMatrix.h>

namespace possumwood {

struct ViewportState {
	ViewportState();

	unsigned width, height;
	Imath::M44f projection, modelview;
};

}
