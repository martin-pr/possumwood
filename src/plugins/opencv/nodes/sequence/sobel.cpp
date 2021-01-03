#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/task_group.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_inSequence;
dependency_graph::InAttr<possumwood::Enum> a_borderType;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_outSequence;

static std::vector<std::pair<std::string, int>> s_borderType = {{"BORDER_DEFAULT", cv::BORDER_DEFAULT},
                                                                {"BORDER_CONSTANT", cv::BORDER_CONSTANT},
                                                                {"BORDER_REPLICATE", cv::BORDER_REPLICATE},
                                                                {"BORDER_REFLECT", cv::BORDER_REFLECT},
                                                                {"BORDER_WRAP", cv::BORDER_WRAP},
                                                                {"BORDER_REFLECT_101", cv::BORDER_REFLECT_101},
                                                                {"BORDER_TRANSPARENT", cv::BORDER_TRANSPARENT},
                                                                {"BORDER_REFLECT101", cv::BORDER_REFLECT101},
                                                                {"BORDER_ISOLATED", cv::BORDER_ISOLATED}};

constexpr float k_smooth(int coord) {
	assert(coord >= -1 && coord <= 1);
	static std::array<float, 3> vals = {1, 2, 1};
	return vals[coord + 1];
}

constexpr float k_diff(int coord) {
	assert(coord >= -1 && coord <= 1);
	static std::array<float, 3> vals = {1, 0, -1};
	return vals[coord + 1];
}

cv::Mat sobel_xy_x(const cv::Mat& m, cv::BorderType border) {
	cv::Mat result = Mat::zeros(m.rows, m.cols, CV_32FC(m.channels));
	for(int y = 0; y < m.rows; ++y)
		for(int x = 0; x < m.cols; ++x) {
			float* val = m.ptr<float>(y, x);
			for(int c = 0; c < m.channels; ++c) {
				for(int yi = -1; yi <= 1; ++yi)
					for(int xi = -1; xi <= 1; ++xi)
						*val += k_smooth(yi) * k_diff(xi) *
						        m.ptr<float>(cv::borderInterpolate(y + yi, m.rows, border),
						                     cv::borderInterpolate(x + xi, m.cols, border))[c];
				++val;
			}
		}
	return result;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& sequence = data.get(a_inSequence);
	possumwood::opencv::Sequence result;

	tbb::task_group group;

	for(auto it = sequence.begin(); it != sequence.end(); ++it)
		group.run([it, &result]() { result[it->first] = cv::sobel_xy_x(it->second, data.get(a_borderType)); });

	group.wait();

	data.set(a_outSequence, possumwood::opencv::Sequence(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inSequence, "in_seq", possumwood::opencv::Sequence());
	meta.addAttribute(a_borderType, "border", possumwood::Enum(s_borderType.begin(), s_borderType.end()));
	meta.addAttribute(a_outSequence, "out_seq", possumwood::opencv::Sequence());

	meta.addInfluence(a_inSequence, a_outSequence);
	meta.addInfluence(a_borderType, a_outSequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/sobel", init);

}  // namespace
