#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "exif_sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::ExifSequence> a_inSequence;
dependency_graph::InAttr<possumwood::opencv::Exif> a_exif;
dependency_graph::OutAttr<possumwood::opencv::ExifSequence> a_outSequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	possumwood::opencv::ExifSequence seq = data.get(a_inSequence);

	seq.add(data.get(a_exif));

	data.set(a_outSequence, seq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_sequence");
	meta.addAttribute(a_exif, "exif", possumwood::opencv::Exif(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outSequence, "out_sequence");

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_exif, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/add_exif", init);

}
