#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "frame.h"
#include "image_loading.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::OutAttr<possumwood::opencv::Exif> a_exif;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	// native reader - cannot read or understand EXIF information
	// data.set(a_frame, possumwood::opencv::Frame(cv::imread(filename.filename().string())));

	auto img = possumwood::opencv::load(filename.filename());

	data.set(a_exif, img.second);
	data.set(a_frame, possumwood::opencv::Frame(img.first));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"Image files (*.png *.jpg *.jpe *.jpeg *.exr *.tif *.tiff)",
	}));
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_exif, "exif", possumwood::opencv::Exif(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_frame);
	meta.addInfluence(a_filename, a_exif);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/image", init);

}
