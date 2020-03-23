#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include <actions/traits.h>

#include "possumwood_sdk/datatypes/enum.h"

#include "sequence.h"
#include "tools.h"

namespace {

// Based on Hu, Xiaoyan, and Philippos Mordohai. "A quantitative evaluation of confidence measures for stereo vision." IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2121-2133.

class Curve {
	public:
		Curve(const possumwood::opencv::Sequence& s, const cv::Vec2i& pos) : m_seq(&s), m_pos(pos) {
			assert(s.size() > 0);
			assert(pos[0] < s[0]->cols);
			assert(pos[1] < s[0]->rows);
		}

		std::size_t size() const {
			return m_seq->size();
		}

		float operator[](std::size_t index) const {
			return (*m_seq)[index]->at<float>(m_pos[1], m_pos[0]);
		}

	private:
		const possumwood::opencv::Sequence* m_seq;
		cv::Vec2i m_pos;
};

float MSM(const Curve& curve) {
	float min = curve[0];
	for(std::size_t i=1; i<curve.size(); ++i)
		min = std::min(min, curve[i]);
	return -min;
}

struct Measure {
	int id;
	std::string name;
	float (*fn)(const Curve&);
};

enum Mode {
	kMSM,
};

static const std::vector<Measure> s_measures {{
	Measure {kMSM, "Matching Score Measure", &MSM}
}};

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& sequence = data.get(a_in);

	if(sequence.size() < 2)
		throw std::runtime_error("At least two images required in the input sequence.");

	const int width = (**sequence.begin()).cols;
	const int height = (**sequence.begin()).rows;

	for(auto& f : sequence)
		if((*f).rows != height || (*f).cols != width)
			throw std::runtime_error("Consistent width and height is required in the input sequence.");

	for(auto& f : sequence)
		if((*f).type() != CV_32FC1)
			throw std::runtime_error("Only CV_32FC1 images accepted on input.");

	float (*fn)(const Curve&) = nullptr;
	for(auto& m : s_measures)
		if(data.get(a_mode).value() == m.name)
			fn = m.fn;
	assert(fn != nullptr);

	cv::Mat result = cv::Mat::zeros(height, width, CV_32FC1);
	tbb::parallel_for(tbb::blocked_range2d<int>(0, height, 0, width), [&](const tbb::blocked_range2d<int>& range) {
		for(int row = range.rows().begin(); row != range.rows().end(); ++row)
			for(int col = range.cols().begin(); col != range.cols().end(); ++col)
				result.at<float>(row, col) = fn(Curve(sequence, cv::Vec2i(col, row)));
	});

	data.set(a_out, possumwood::opencv::Frame(result));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_sequence", possumwood::opencv::Sequence());

	std::vector<std::pair<std::string, int>> options;
	for(auto& m : s_measures)
		options.push_back(std::make_pair(m.name, m.id));
	meta.addAttribute(a_mode, "mode", possumwood::Enum(options.begin(), options.end()));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/sequence/confidence", init);

}
