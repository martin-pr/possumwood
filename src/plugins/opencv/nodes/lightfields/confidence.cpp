#include <actions/traits.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include "possumwood_sdk/datatypes/enum.h"
#include "sequence.h"
#include "tools.h"

namespace {

// Based on Hu, Xiaoyan, and Philippos Mordohai. "A quantitative evaluation of confidence measures for stereo vision."
// IEEE transactions on pattern analysis and machine intelligence 34.11 (2012): 2121-2133.

class Curve {
  public:
	Curve(const possumwood::opencv::Sequence& s, const cv::Vec2i& pos) : m_seq(&s), m_pos(pos) {
		assert(s.size() > 0);
		assert(pos[0] < s(0).cols);
		assert(pos[1] < s(0).rows);
	}

	bool empty() const {
		return m_seq->empty();
	}

	std::size_t size() const {
		return m_seq->size();
	}

	float operator[](std::size_t index) const {
		return get(index);
	}

	// returns the minimum element index
	std::size_t d_1() const {
		assert(!empty());

		float min = get(0);
		std::size_t min_index = 0;

		for(std::size_t i = 1; i < size(); ++i)
			if(min > get(i)) {
				min_index = i;
				min = get(i);
			}

		return min_index;
	}

	// returns the minimum element value
	float c_1() const {
		return get(d_1());
	}

	// returns the second minimum element index
	std::size_t d_2() const {
		assert(!empty());

		float min = std::numeric_limits<float>::max();
		std::size_t min_index = 0;

		float min_2 = min;
		std::size_t min_2_index = 0;

		for(std::size_t i = 0; i < size(); ++i) {
			const float current = get(i);

			if(min > current) {
				min_2_index = min_index;
				min_2 = min;

				min_index = i;
				min = current;
			}
			else if(min_2 > current) {
				min_2_index = i;
				min_2 = current;
			}
		}

		if(min_2 < std::numeric_limits<float>::max())
			return min_2_index;
		return min_index;
	}

	// returns the second minimum element value
	float c_2() const {
		return get(d_2());
	}

	// returns the second local minimum element index
	std::size_t d_2m() const {
		assert(!empty());

		float min = std::numeric_limits<float>::max();
		std::size_t min_index = 0;

		float min_2 = min;
		std::size_t min_2_index = 0;

		for(std::size_t i = 0; i < size(); ++i)
			if(isPeak(i)) {
				const float current = get(i);

				if(min > current) {
					min_2_index = min_index;
					min_2 = min;

					min_index = i;
					min = current;
				}
				else if(min_2 > current) {
					min_2_index = i;
					min_2 = current;
				}
			}

		if(min_2 < std::numeric_limits<float>::max())
			return min_2_index;
		return min_index;
	}

	// returns the second minimum element value
	float c_2m() const {
		return get(d_2m());
	}

	std::pair<float, float> minmax() const {
		float min = get(0);
		float max = get(0);

		for(std::size_t i = 1; i < size(); ++i) {
			min = std::min(min, get(i));
			max = std::max(max, get(i));
		}

		return std::make_pair(min, max);
	}

	float sum() const {
		float result = 0.0f;
		for(std::size_t i = 1; i < size(); ++i)
			result += get(i);
		return result;
	}

  private:
	float get(std::size_t index) const {
		return (*m_seq)(index).at<float>(m_pos[1], m_pos[0]);
	}

	bool isPeak(std::size_t index) const {
		bool result = true;
		if(index > 0)
			result &= get(index - 1) > get(index);
		if(index + 1 < size())
			result &= get(index + 1) > get(index);
		return result;
	}

	const possumwood::opencv::Sequence* m_seq;
	cv::Vec2i m_pos;
};

/////////////////////

float MSM(const Curve& curve, float) {
	return -curve.c_1();
}

float CUR(const Curve& curve, float) {
	assert(curve.size() >= 2);
	const std::size_t d_1 = curve.d_1();

	if(d_1 == 0)
		return -2.0f * curve[d_1] + 2.0f * curve[d_1 + 1];
	else if(d_1 + 1 == curve.size())
		return -2.0f * curve[d_1] + 2.0f * curve[d_1 - 1];
	else
		return -2.0f * curve[d_1] + curve[d_1 - 1] + curve[d_1 + 1];
}

float PKR(const Curve& curve, float) {
	if(curve.c_1() > 0.0f)
		return curve.c_2m() / curve.c_1();
	return 0.0f;
}

float PKRN(const Curve& curve, float) {
	if(curve.c_1() > 0.0f)
		return curve.c_2() / curve.c_1();
	return 0.0f;
}

float MMN(const Curve& curve, float) {
	return curve.c_2() - curve.c_1();
}

float PRB(const Curve& curve, float) {
	return -curve.c_1() / curve.sum();
}

float MLM(const Curve& curve, float sigma) {
	const float val = std::exp(-curve.c_1() / (2 * sigma * sigma));

	float sum = 0.0f;
	for(std::size_t a = 0; a < curve.size(); ++a)
		sum += std::exp(-curve[a] / (2 * sigma * sigma));

	return val / sum;
}

float AML(const Curve& curve, float sigma) {
	const float c1 = curve.c_1();

	float sum = 0.0f;
	for(std::size_t a = 0; a < curve.size(); ++a)
		sum += std::exp(-std::pow(curve[a] - c1, 2) / (2 * sigma * sigma));

	return 1.0f / sum;
}

float NEM(const Curve& curve, float sigma) {
	float p_denom = 0.0f;
	for(std::size_t a = 0; a < curve.size(); ++a)
		p_denom += std::exp(-curve[a]);
	assert(p_denom > 0.0f);

	float result = 0;
	for(std::size_t a = 0; a < curve.size(); ++a) {
		assert(curve[a] >= 0.0f);
		assert(std::exp(-curve[a]) >= 0.0f);

		const float p = std::exp(-curve[a]) / p_denom;
		assert(p >= 0.0f);
		if(p > 0.0f)
			result += p * std::log(p);
	}

	return result;
}

float NOI(const Curve& curve, float sigma) {
	// low pass filter
	std::vector<float> vals(curve.size());
	std::vector<float> norms(curve.size());

	const int delta = std::ceil(sigma * 3.0f);

	for(int i = 0; i < int(curve.size()); ++i)
		for(int o = std::max(i - delta, 0); i <= std::min(i + delta, int(curve.size() - 1)); ++i) {
			const float w = std::exp(-0.5f * std::pow(float(i - o) / sigma, 2));

			norms[i] += w;
			vals[i] += curve[i] * w;
		}

	for(int i = 0; i < int(curve.size()); ++i)
		vals[i] /= norms[i];

	// inflection points counting
	int count = 0;
	if(vals[0] < vals[1])
		++count;
	if(vals[vals.size() - 1] < vals[vals.size() - 2])
		++count;
	for(std::size_t a = 1; a < vals.size() - 1; ++a)
		if(vals[a - 1] > vals[a] && vals[a] < vals[a + 1])
			++count;

	return -count;
}

float WMN(const Curve& curve, float) {
	return (curve.c_2m() - curve.c_1()) / curve.sum();
}

float WMNN(const Curve& curve, float) {
	return (curve.c_2() - curve.c_1()) / curve.sum();
}

struct Measure {
	int id;
	std::string name;
	float (*fn)(const Curve&, float);
};

enum Mode { kMSM, kCUR, kPKR, kPKRN, kMMN, kPRB, kMLM, kAML, kNEM, kNOI, kWMN, kWMNN };

static const std::vector<Measure> s_measures{{
    Measure{kMSM, "Matching Score Measure", &MSM},
    Measure{kCUR, "Curvature", &CUR},
    Measure{kPKR, "Peak Ratio", &PKR},
    Measure{kPKRN, "Peak Ratio (naive)", &PKRN},
    Measure{kMMN, "Maximum Margin", &MMN},
    Measure{kPRB, "Probabilistic Measure", &PRB},
    Measure{kMLM, "Maximum Likelihood Measure", &MLM},
    Measure{kAML, "Attainable Maximum Likelihood", &AML},
    Measure{kNEM, "Negative Entropy Measure", &NEM},
    Measure{kNOI, "Number of Inflections (filtered)", &NOI},
    Measure{kWMN, "Winner Margin", &WMN},
    Measure{kWMNN, "Winner Margin (naive)", &WMNN},
}};

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<float> a_sigma;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& sequence = data.get(a_in);

	if(sequence.size() < 2)
		throw std::runtime_error("At least two images required in the input sequence.");

	const int width = sequence.meta().cols;
	const int height = sequence.meta().rows;

	if(sequence.meta().type != CV_32FC1)
		throw std::runtime_error("Only CV_32FC1 images accepted on input.");

	float (*fn)(const Curve&, float) = nullptr;
	for(auto& m : s_measures)
		if(data.get(a_mode).value() == m.name)
			fn = m.fn;
	assert(fn != nullptr);

	cv::Mat result = cv::Mat::zeros(height, width, CV_32FC1);
	tbb::parallel_for(tbb::blocked_range2d<int>(0, height, 0, width), [&](const tbb::blocked_range2d<int>& range) {
		const float sigma = data.get(a_sigma);
		for(int row = range.rows().begin(); row != range.rows().end(); ++row)
			for(int col = range.cols().begin(); col != range.cols().end(); ++col)
				result.at<float>(row, col) = fn(Curve(sequence, cv::Vec2i(col, row)), sigma);
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
	meta.addAttribute(a_sigma, "sigma", 1.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_mode, a_out);
	meta.addInfluence(a_sigma, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/confidence", init);

}  // namespace
