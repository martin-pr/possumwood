#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filenames.h>

#include <tbb/parallel_for.h>

#include <actions/traits.h>

#include "sequence.h"
#include "image_loading.h"

namespace {

dependency_graph::InAttr<possumwood::Filenames> a_filenames;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_seq;

dependency_graph::State compute(dependency_graph::Values& data) {
	const auto filenames = data.get(a_filenames).filenames();

	possumwood::opencv::Sequence seq(filenames.size());

	tbb::parallel_for(std::size_t(0), filenames.size(), [&](std::size_t i) {
		auto img = possumwood::opencv::load(filenames[i]);
		seq[i] = img.first;
	});

	data.set(a_seq, seq);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filenames, "filenames", possumwood::Filenames({
		"Image files (*.png *.jpg *.jpe *.jpeg *.exr *.tif *.tiff)",
	}));
	meta.addAttribute(a_seq, "sequence");

	meta.addInfluence(a_filenames, a_seq);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/image_sequence", init);

}
