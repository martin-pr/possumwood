#include "viewport_state.h"

namespace possumwood {

ViewportState::ViewportState() : width(300), height(200) {
	projection.makeIdentity();
	modelview.makeIdentity();
}

}
