#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/app.h>
#include <possumwood_sdk/config.inl>

namespace {

dependency_graph::OutAttr<unsigned> a_frame;

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame");

	meta.setCompute([](dependency_graph::Values& vals) {
		vals.set(a_frame, static_cast<unsigned>(std::round(
			possumwood::App::instance().time() * 
			possumwood::App::instance().sceneConfig()["fps"].as<float>())));

		return dependency_graph::State();
	});
}

/// This is a hack - "frame" nodes have a single output, set by a callback in the
/// possumwood SDK's App object
possumwood::NodeImplementation s_impl("frame", init);

}
