#include <possumwood_sdk/node_implementation.h>

#include <lua/datatypes/context.h>
#include <lua/datatypes/variable.inl>

#include "lua/images.h"
#include "lua/pixmap_wrapper.h"

#include "lua/nodes/inject.h"

namespace {

template<typename PIXMAP, typename SUFFIX>
struct Module {
	static std::string name() {
		return std::string("images") + SUFFIX::value();
	}

	static void init(possumwood::lua::State& state) {
		possumwood::images::init<PIXMAP>(state, SUFFIX::value());
	}
};

// suffix as a class, so it can be used as a template parameter
struct LDRSuffix {
	static const std::string value() {
		return "";
	};
};

struct HDRSuffix {
	static const std::string value() {
		return "_hdr";
	};
};

possumwood::NodeImplementation s_impl_ldr("lua/inject/image",
	[](possumwood::Metadata& meta) {
		possumwood::lua::Inject<
			std::shared_ptr<const possumwood::LDRPixmap>,
			possumwood::images::PixmapWrapper<possumwood::LDRPixmap>,
			Module<possumwood::LDRPixmap, LDRSuffix>
		>::init(meta);
	}
);

possumwood::NodeImplementation s_impl_hdr("lua/inject/image_hdr",
	[](possumwood::Metadata& meta) {
		possumwood::lua::Inject<
			std::shared_ptr<const possumwood::HDRPixmap>,
			possumwood::images::PixmapWrapper<possumwood::HDRPixmap>,
			Module<possumwood::HDRPixmap, HDRSuffix>
		>::init(meta);
	}
);

}
