#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <actions/traits.h>
#include <possumwood_sdk/datatypes/enum.h>

#include "frame.h"

namespace {

struct MeanAttrs {
	virtual ~MeanAttrs() = default;

	dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
	dependency_graph::InAttr<possumwood::opencv::Frame> a_superpixels;
	dependency_graph::InAttr<possumwood::Enum> a_mode;
	dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

	virtual std::function<float(int, int)> weightFn(dependency_graph::Values& data) const;
	void init(possumwood::Metadata& meta);

	dependency_graph::State compute(dependency_graph::Values& data) const;
};

struct WeightedMeanAttrs : public MeanAttrs {
	dependency_graph::InAttr<possumwood::opencv::Frame> a_weights;

	virtual std::function<float(int, int)> weightFn(dependency_graph::Values& data) const override;
	void init(possumwood::Metadata& meta);
};

MeanAttrs s_meanAttrs;
WeightedMeanAttrs s_weightedMeanAttrs;

float getFloat(const cv::Mat& in, int row, int col, int channel) {
	switch(in.depth()) {
		case CV_8U: return in.ptr<uint8_t>(row, col)[channel];
		case CV_8S: return in.ptr<int8_t>(row, col)[channel];
		case CV_16U: return in.ptr<uint16_t>(row, col)[channel];
		case CV_16S: return in.ptr<int16_t>(row, col)[channel];
		case CV_32S: return in.ptr<int32_t>(row, col)[channel];
		case CV_32F: return in.ptr<float>(row, col)[channel];
		case CV_64F: return in.ptr<double>(row, col)[channel];
		default: throw std::runtime_error("Unknown number type in input.");
	}
}

void setFloat(cv::Mat& in, int row, int col, int channel, float val) {
	switch(in.depth()) {
		case CV_8U: in.ptr<uint8_t>(row, col)[channel] = val; break;
		case CV_8S: in.ptr<int8_t>(row, col)[channel] = val; break;
		case CV_16U: in.ptr<uint16_t>(row, col)[channel] = val; break;
		case CV_16S: in.ptr<int16_t>(row, col)[channel] = val; break;
		case CV_32S: in.ptr<int32_t>(row, col)[channel] = val; break;
		case CV_32F: in.ptr<float>(row, col)[channel] = val; break;
		case CV_64F: in.ptr<double>(row, col)[channel] = val; break;
		default: throw std::runtime_error("Unknown number type in input.");
	}
}

enum Mode {
	kMean,
	kMedian,
};

static std::vector<std::pair<std::string, int>> s_mode {
	{"mean", kMean},
	{"median", kMedian},
};

class Mean {
	public:
		Mean() : m_val(0.0f), m_norm(0.0f) {
		}

		void add(float val, float weight) {
			if(weight < 0.0f)
				throw std::runtime_error("Negative weight in weights input!");

			m_val += val * weight;
			m_norm += weight;
		}

		float operator*() const {
			if(m_norm > 0.0f)
				return m_val / m_norm;
			return 0.0f;
		}

	private:
		float m_val, m_norm;
};

class Median {
	public:
		Median() : m_weightsSum(0.0f) {
		}

		void add(float val, float weight) {
			if(weight < 0.0f)
				throw std::runtime_error("Negative weight in weights input!");

			m_val.push_back(std::make_pair(val, weight));
			m_weightsSum += weight;
		}

		float operator*() {
			if(m_weightsSum == 0.0f)
				return 0.0f;

			std::sort(m_val.begin(), m_val.end());

			auto it = m_val.begin();
			float weight = 0.0f;
			weight += it->second;

			while(weight < m_weightsSum/2.0f && it != m_val.end()-1) {
				++it;

				if(it != m_val.end())
					weight += it->second;
			}

			return it->first;
		}

	private:
		std::vector<std::pair<float, float>> m_val;
		float m_weightsSum;
};

template<typename MEAN>
cv::Mat process(const cv::Mat& in, const cv::Mat& superpixels, std::function<float(int, int)> weights = [](int, int) {return 1.0f;}) {
	// first of all, get the maximum index of the superpixels
	int32_t maxIndex = 0;
	for(int row=0; row<superpixels.rows; ++row)
		for(int col=0; col<superpixels.cols; ++col)
			maxIndex = std::max(maxIndex, superpixels.at<int32_t>(row, col));

	// make the right sized accumulator and norm array
	std::vector<std::vector<Mean>> vals(in.channels(), std::vector<Mean>(maxIndex+1, Mean()));

	// and accumulate the values
	for(int row=0; row<in.rows; ++row)
		for(int col=0; col<in.cols; ++col) {
			const int32_t index = superpixels.at<int32_t>(row, col);

			for(int c=0;c<in.channels();++c)
				vals[c][index].add(getFloat(in, row, col, c), weights(row, col));
		}

	cv::Mat out = cv::Mat::zeros(in.rows, in.cols, in.type());

	// assign the result
	for(int row=0; row<in.rows; ++row)
		for(int col=0; col<in.cols; ++col)
			for(int c=0;c<in.channels();++c) {
				const int32_t index = superpixels.at<int32_t>(row, col);
				setFloat(out, row, col, c, *vals[c][index]);
			}

	return out;
}

std::function<float(int, int)> MeanAttrs::weightFn(dependency_graph::Values& data) const {
	return [](int, int) { return 1.0f; };
}

std::function<float(int, int)> WeightedMeanAttrs::weightFn(dependency_graph::Values& data) const {
	const cv::Mat& in = *data.get(a_in);
	const cv::Mat& weights = *data.get(a_weights);

	if(in.rows != weights.rows || in.cols != weights.cols)
		throw std::runtime_error("Input and weights size have to match.");

	if(weights.type() != CV_32FC1)
		throw std::runtime_error("Only CV_32FC1 type supported on the weights input!");

	return [&](int row, int col) {
		return weights.at<float>(row, col);
	};
}

dependency_graph::State MeanAttrs::compute(dependency_graph::Values& data) const {
	const cv::Mat& in = *data.get(a_in);
	const cv::Mat& superpixels = *data.get(a_superpixels);

	if(in.rows != superpixels.rows || in.cols != superpixels.cols)
		throw std::runtime_error("Input and superpixel size have to match.");

	if(superpixels.type() != CV_32SC1)
		throw std::runtime_error("Only CV_32SC1 type supported on the superpixels input!");

	std::function<float(int, int)> weights = weightFn(data);

	cv::Mat out;
	if(data.get(a_mode).intValue() == kMean)
		out = process<Mean>(in, superpixels, weights);
	else if(data.get(a_mode).intValue() == kMedian)
		out = process<Median>(in, superpixels, weights);
	else
		throw std::runtime_error("Unknown mode " + data.get(a_mode).value());

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void MeanAttrs::init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_superpixels, "superpixels", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mode, "mode", possumwood::Enum(s_mode.begin(), s_mode.end()));
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_superpixels, a_out);
	meta.addInfluence(a_mode, a_out);

	meta.setCompute([this](dependency_graph::Values& data) {
		return compute(data);
	});
}

void WeightedMeanAttrs::init(possumwood::Metadata& meta) {
	MeanAttrs::init(meta);

	meta.addAttribute(a_weights, "weights", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_weights, a_out);

	meta.setCompute([this](dependency_graph::Values& data) {
		return compute(data);
	});
}

possumwood::NodeImplementation s_impl("opencv/superpixels/mean", [](possumwood::Metadata& meta) { s_meanAttrs.init(meta); });
possumwood::NodeImplementation s_implW("opencv/superpixels/weighted_mean", [](possumwood::Metadata& meta) { s_weightedMeanAttrs.init(meta); });

}
