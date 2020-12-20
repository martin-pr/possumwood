#include <tbb/blocked_range2d.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

#include <possumwood_sdk/node_implementation.h>

#include <actions/traits.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<unsigned> a_kernelSize;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

template <typename T>
class MedianFilter {
  public:
	explicit MedianFilter(int size) : m_size(size / 2), m_buffer(std::vector<T>(size * size)) {
		assert(size % 2 == 1);
	}

	T operator()(const cv::Mat& m, int x, int y, int channel) {
		auto& buffer = m_buffer.local();

		buffer.clear();
		for(int yi = std::max(0, y - m_size); yi <= std::min(y + m_size, m.cols - 1); ++yi)
			for(int xi = std::max(0, x - m_size); xi <= std::min(x + m_size, m.rows - 1); ++xi) {
				buffer.push_back(m.ptr<T>(yi, xi)[channel]);
			}

		std::sort(buffer.begin(), buffer.end());

		return buffer[buffer.size() / 2];
	}

  private:
	int m_size;
	tbb::enumerable_thread_specific<std::vector<T>> m_buffer;
};

template <typename T>
void doTheThing(const possumwood::opencv::Sequence& input, possumwood::opencv::Sequence& result, int kernelSize) {
	tbb::task_group group;

	for(auto it = input.begin(); it != input.end(); ++it) {
		group.run([it, &result, kernelSize]() {
			const cv::Mat& in = it->second;
			cv::Mat out = cv::Mat::zeros(in.rows, in.cols, in.type());

			MedianFilter<T> filter(kernelSize);

			tbb::parallel_for(tbb::blocked_range2d<int>(0, in.rows, 0, in.cols),
			                  [&](const tbb::blocked_range2d<int>& range) {
				                  for(int y = range.rows().begin(); y != range.rows().end(); ++y)
					                  for(int x = range.cols().begin(); x != range.cols().end(); ++x)
						                  for(int c = 0; c < in.channels(); ++c)
							                  out.ptr<T>(y, x)[c] = filter(in, x, y, c);
			                  });

			result[it->first] = std::move(out);
		});
	}

	group.wait();
}

dependency_graph::State compute(dependency_graph::Values& data) {
	if(data.get(a_kernelSize) % 2 != 1)
		throw std::runtime_error("Median filter size should be an odd number.");

	const possumwood::opencv::Sequence& input = data.get(a_inSequence);
	possumwood::opencv::Sequence result;

	if(!input.empty()) {
		switch(input.begin()->second.depth()) {
			case CV_32F:
				doTheThing<float>(input, result, data.get(a_kernelSize));
				break;
			case CV_8U:
				doTheThing<uint8_t>(input, result, data.get(a_kernelSize));
				break;
			case CV_16U:
				doTheThing<uint16_t>(input, result, data.get(a_kernelSize));
				break;
			default:
				throw std::runtime_error("Unsupported sequence type on input");
		}
	}

	data.set(a_outSequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_kernelSize, "kernel_size", 5u);
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_kernelSize, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/median", init);

}  // namespace
