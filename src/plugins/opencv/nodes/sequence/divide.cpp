#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_seq1, a_seq2;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq1 = data.get(a_seq1);
	const possumwood::opencv::Sequence& seq2 = data.get(a_seq2);

	if(seq1.empty() || seq2.empty())
		throw std::runtime_error("One or both sequences are empty.");

	if(!possumwood::opencv::Sequence::hasMatchingKeys(seq1, seq2) && !seq1.hasOneElement() && !seq2.hasOneElement())
		throw std::runtime_error("Sequences need to be the same size, or one of them needs to be of size 1.");

	possumwood::opencv::Sequence result;

	tbb::task_group group;

	auto it1 = seq1.begin();
	auto it2 = seq2.begin();

	while(it1 != seq1.end() && it2 != seq2.end()) {
		group.run([&seq1, &result, it1, it2]() {
			cv::Mat m;
			cv::divide(it1->second, it2->second, m);

			if(seq1.hasOneElement())
				result[it2->first] = std::move(m);
			else
				result[it1->first] = std::move(m);
		});

		if(seq1.hasOneElement())
			++it2;
		else
			++it1;
	}

	group.wait();

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

possumwood::NodeImplementation s_impl("opencv/sequence/divide", init);

}  // namespace
