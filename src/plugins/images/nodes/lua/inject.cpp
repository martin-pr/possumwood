#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/images.h"
#include "lua/opencv_image.h"

#include "lua/nodes/inject.h"

#include "opencv/frame.h"

namespace {

template<typename PIXMAP, typename SUFFIX, typename WRAPPER>
struct Module {
	static std::string name() {
		return std::string("images") + SUFFIX::value();
	}

	static void init(possumwood::lua::State& state) {
		possumwood::images::init<PIXMAP, WRAPPER>(state, SUFFIX::value());
	}
};

struct CVSuffix {
	static const std::string value() {
		return "_opencv";
	};
};

struct CVHDRSuffix {
	static const std::string value() {
		return "_opencv_hdr";
	};
};

possumwood::NodeImplementation s_impl_opencv("lua/inject/opencv_image",
	[](possumwood::Metadata& meta) {
		possumwood::lua::Inject<
			possumwood::opencv::Frame,
			possumwood::images::OpencvMatWrapper<CV_8U>,
			Module<possumwood::opencv::Frame, CVSuffix, possumwood::images::OpencvMatWrapper<CV_8U>>
		>::init(meta);
	}
);

possumwood::NodeImplementation s_impl_opencv_hdr("lua/inject/opencv_image_hdr",
	[](possumwood::Metadata& meta) {
		possumwood::lua::Inject<
			possumwood::opencv::Frame,
			possumwood::images::OpencvMatWrapper<CV_32F>,
			Module<possumwood::opencv::Frame, CVHDRSuffix, possumwood::images::OpencvMatWrapper<CV_32F>>
		>::init(meta);
	}
);


}
