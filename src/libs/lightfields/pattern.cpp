#include "pattern.h"

#include <OpenEXR/ImathMatrix.h>

namespace lightfields {

Pattern::Pattern(const Imath::V2i& resolution) : m_sensorResolution(resolution) {
}

Pattern::~Pattern() {
}

const Imath::V2i& Pattern::sensorResolution() const {
	return m_sensorResolution;
}

}
