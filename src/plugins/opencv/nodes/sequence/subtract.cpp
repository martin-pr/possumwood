#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include <tbb/parallel_for.h>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_seq1, a_seq2;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq1 = data.get(a_seq1);
	const possumwood::opencv::Sequence& seq2 = data.get(a_seq2);

	if(seq1.size() == 0 || seq2.size() == 0)
		throw std::runtime_error("One of the sequences is empty.");

	if(seq1.size() != seq2.size() && seq1.size() != 1 && seq2.size() != 1)
		throw std::runtime_error("Sequences need to be the same size, or one of them needs to be of size 1.");

	possumwood::opencv::Sequence result(std::max(seq1.size(), seq2.size()));
	tbb::parallel_for(std::size_t(0), result.size(), [&](std::size_t index) {
		cv::subtract(*seq1[std::min(index, seq1.size()-1)], *seq2[std::min(index, seq2.size()-1)], *result[index]);
	});

	data.set(a_out, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_seq1, "sequence_1");
	meta.addAttribute(a_seq2, "sequence_2");
	meta.addAttribute(a_out, "out");

	meta.addInfluence(a_seq1, a_out);
	meta.addInfluence(a_seq2, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/subtract", init);

}
