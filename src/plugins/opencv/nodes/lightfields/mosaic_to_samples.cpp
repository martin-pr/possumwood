#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

#include "lightfields.h"
#include "sequence.h"

namespace {

using SeqRange = tbb::blocked_range<possumwood::opencv::Sequence::const_iterator>;

dependency_graph::InAttr<possumwood::opencv::Sequence> a_sequence;
dependency_graph::OutAttr<lightfields::Samples> a_samples;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& seq = data.get(a_sequence);

	lightfields::Samples samples(Imath::V2i(seq.meta().rows, seq.meta().cols));
	samples.resize(std::size_t(seq.meta().rows) * std::size_t(seq.meta().cols) * seq.size());

	tbb::task_group tasks;

	std::size_t ctr = 0;
	for(auto it = seq.begin(); it != seq.end(); ++it, ++ctr) {
		tasks.run([it, &samples, &seq, ctr]() {
			Imath::V2f uv(0.0f, 0.0f);
			if(seq.max().x != seq.min().x)
				uv.x = (float(it->first.x) - seq.min().x) / float(seq.max().x - seq.min().x) * 2.0f - 1.0f;
			if(seq.max().y != seq.min().y)
				uv.y = (float(it->first.y) - seq.min().y) / float(seq.max().y - seq.min().y) * 2.0f - 1.0f;

			tbb::parallel_for(tbb::blocked_range<int>(0, seq.meta().rows), [&](const tbb::blocked_range<int>& range) {
				auto target =
				    samples.begin() + ctr * seq.meta().rows * seq.meta().cols + range.begin() * seq.meta().cols;

				for(int y = range.begin(); y != range.end(); ++y)
					for(int x = 0; x < seq.meta().cols; ++x) {
						target->xy.x = (float)x;
						target->xy.y = (float)y;

						target->uv = uv;

						target->color = lightfields::Samples::kRGB;

						const float* ptr = it->second.ptr<float>(y, x);
						for(int a = 0; a < 3; ++a)
							target->value[a] = ptr[a];

						++target;
					}
			});
		});
	}

	tasks.wait();

	data.set(a_samples, std::move(samples));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_sequence, "sequence");
	meta.addAttribute(a_samples, "samples", lightfields::Samples(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_sequence, a_samples);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/mosaic_to_samples", init);

}  // namespace
