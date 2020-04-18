#include "slic_superpixels.h"

#include "bitfield.h"

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

namespace lightfields {

SlicSuperpixels::Center::Center() : row(0), col(0), color{{0, 0, 0}} {
}

SlicSuperpixels::Center::Center(const cv::Mat& m, int r, int c) : row(r), col(c) {
	assert(r >= 0 && r < m.rows);
	assert(c >= 0 && c < m.cols);

	auto ptr = m.ptr<unsigned char>(r, c);
	for(int a=0;a<3;++a)
		color[a] = ptr[a];
}

SlicSuperpixels::Center& SlicSuperpixels::Center::operator +=(const Center& c) {
	row += c.row;
	col += c.col;

	for(int a=0;a<3;++a)
		color[a] += c.color[a];

	return *this;
}

SlicSuperpixels::Center& SlicSuperpixels::Center::operator /=(int div) {
	row /= div;
	col /= div;

	for(int a=0;a<3;++a)
		color[a] /= div;

	return *this;
}

////////////

SlicSuperpixels::Metric::Metric(int S, float m) : m_S(S), m_SS(S*S), m_mm(m*m) {
}

float SlicSuperpixels::Metric::operator()(const lightfields::SlicSuperpixels::Center& c, const cv::Mat& m, const int row, const int col) const {
	float d_c = 0.0f;
	for(int a=0;a<3;++a) {
		float elem = float(c.color[a]) - float(m.ptr<unsigned char>(row, col)[a]);
		elem *= elem;

		d_c += elem;
	}

	const float d_s = float(c.row - row)*float(c.row - row) + float(c.col - col)*float(c.col - col);

	return std::sqrt(d_c + d_s / m_SS * m_mm);
}

int SlicSuperpixels::Metric::S() const {
	return m_S;
}

////////////

int SlicSuperpixels::initS(int rows, int cols, int pixelCount) {
	int result = std::sqrt((cols * rows) / pixelCount);
	if(result <= 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	return result;
}

lightfields::Grid<lightfields::SlicSuperpixels::Center> SlicSuperpixels::initPixels(const cv::Mat& in, int S) {
	const int rows = in.rows / S;
	const int cols = in.cols / S;

	lightfields::Grid<lightfields::SlicSuperpixels::Center> pixels(rows, cols);

	for(int y=0; y<rows; ++y)
		for(int x=0; x<cols; ++x)
			pixels(y, x) = lightfields::SlicSuperpixels::Center(in, (in.rows * y) / rows + S/2, (in.cols * x) / cols + S/2);

	return pixels;
}

namespace {
	void labelRange(const tbb::blocked_range2d<int>& range, const cv::Mat& in, Grid<SlicSuperpixels::Label>& labels, const Grid<SlicSuperpixels::Center>& centers, const SlicSuperpixels::Metric& metric) {
		for(std::size_t center_id = 0; center_id < centers.container().size(); ++center_id) {
			const lightfields::SlicSuperpixels::Center& center = centers.container()[center_id];

			for(int y = std::max(range.rows().begin(), center.row - metric.S()); y < std::min(range.rows().end(), center.row + metric.S()+1); ++y)
				for(int x = std::max(range.cols().begin(), center.col - metric.S()); x < std::min(range.cols().end(), center.col + metric.S()+1); ++x) {
					const float dist = metric(center, in, y, x);
					SlicSuperpixels::Label& current = labels(y, x);

					if(current.metric > dist)
						current = SlicSuperpixels::Label(center_id, dist);
				}
		}
	}
}

void SlicSuperpixels::label(const cv::Mat& in, lightfields::Grid<Label>& labels, const lightfields::Grid<lightfields::SlicSuperpixels::Center>& centers, const Metric& metric) {
	assert(in.rows == (int)labels.rows() && in.cols == (int)labels.cols());

	// clear all labels
	tbb::parallel_for(0, int(labels.container().size()), [&](int a) {
		labels.container()[a] = Label();
	});

	// using the metric instance, label all pixels
	tbb::parallel_for(tbb::blocked_range2d<int>(0, in.rows, 0, in.cols), [&](const tbb::blocked_range2d<int>& range) {
		labelRange(range, in, labels, centers, metric);
	});
}

void SlicSuperpixels::findCenters(const cv::Mat& in, const lightfields::Grid<Label>& labels, lightfields::Grid<lightfields::SlicSuperpixels::Center>& centers) {
	std::vector<lightfields::SlicSuperpixels::Center> sum(centers.container().size());
	std::vector<int> count(centers.container().size(), 0);

	for(int row=0; row<in.rows; ++row)
		for(int col=0; col<in.cols; ++col) {
			const int label = labels(row, col).id;
			assert(label >= 0 && label < (int)sum.size());

			sum[label] += lightfields::SlicSuperpixels::Center(in, row, col);
			count[label]++;
		}

	for(std::size_t a=0; a<sum.size(); ++a)
		if(count[a] > 0) {
			sum[a] /= count[a];
			centers.container()[a] = sum[a];
		}
}

void SlicSuperpixels::connectedComponents(lightfields::Grid<Label>& labels, const lightfields::Grid<lightfields::SlicSuperpixels::Center>& centers) {
	// first, find all components for each label
	//  label - component - pixels (pair of ints)
	std::vector<std::vector<std::vector<std::pair<int, int>>>> components(centers.container().size());

	{
		lightfields::Grid<bool, lightfields::Bitfield> connected(labels.rows(), labels.cols());

		for(int y=0; y<(int)labels.rows(); ++y)
			for(int x=0; x<(int)labels.cols(); ++x) {
				// found a new component
				if(!connected(y, x)) {
					// recursively map it
					const int currentLabel = labels(y, x).id;

					components[currentLabel].push_back(std::vector<std::pair<int, int>>());
					std::vector<std::pair<int, int>>& currentComponent = components[currentLabel].back();

					std::vector<std::pair<int, int>> stack;
					stack.push_back(std::make_pair(y, x));

					while(!stack.empty()) {
						const std::pair<int, int> current = stack.back();
						stack.pop_back();

						assert(current.first < (int)labels.rows());
						assert(current.second < (int)labels.cols());

						if(!connected(current.first, current.second) && labels(current.first, current.second).id == currentLabel) {
							connected(current.first, current.second) = true;

							currentComponent.push_back(std::make_pair(current.first, current.second));

							if(current.first > 0)
								stack.push_back(std::make_pair(current.first-1, current.second));
							if(current.first < (int)labels.rows()-1)
								stack.push_back(std::make_pair(current.first+1, current.second));
							if(current.second > 0)
								stack.push_back(std::make_pair(current.first, current.second-1));
							if(current.second < (int)labels.cols()-1)
								stack.push_back(std::make_pair(current.first, current.second+1));
						}
					}
				}
			}
	}

	// the largest component is THE component for the label - let's remove it from the list of components to process
	lightfields::Grid<bool, lightfields::Bitfield> processed(labels.rows(), labels.cols());
	for(std::size_t a=0; a<components.size(); ++a) {
		auto& label = components[a];

		if(!label.empty()) {
			std::size_t maxSize = 0;
			std::size_t maxIndex = 0;

			for(int i=0; i<(int)label.size(); ++i)
				if(label[i].size() > maxSize) {
					maxIndex = i;
					maxSize = label[i].size();
				}

			// mark the component as "processed"
			for(auto& p : label[maxIndex])
				processed(p.first, p.second) = true;

			// and then remove the component from the label list
			if(maxSize > 0)
				label.erase(label.begin() + maxIndex);
		}
	}

	// finally, go through all the remaining components and relabel them based on surrounding labels
	// -> in some cases, the unlabelled components can be "nested" (especially in the presence of noise).
	//    We need to run this algorithm iteratively until we managed to label everything.
	std::size_t unprocessed = 1;
	while(unprocessed > 0) {
		unprocessed = 0;

		for(std::size_t label_id=0; label_id<components.size(); ++label_id) {
			auto& label = components[label_id];

			for(auto& comp : label) {
				std::vector<unsigned> counter(components.size());

				for(auto& p : comp) {
					if(p.first > 0 && processed(p.first-1, p.second))
						counter[labels(p.first-1, p.second).id]++;
					if(p.first < (int)labels.rows()-1 && processed(p.first+1, p.second))
						counter[labels(p.first+1, p.second).id]++;
					if(p.second > 0 && processed(p.first, p.second-1))
						counter[labels(p.first, p.second-1).id]++;
					if(p.second < (int)labels.cols()-1 && processed(p.first, p.second+1))
						counter[labels(p.first, p.second+1).id]++;
				}

				// find the most prelevant resolved label (there might not be any!)
				std::size_t maxLabel = 0;
				unsigned maxCount = 0;

				for(std::size_t a=0;a<counter.size();++a)
					if(maxCount < counter[a]) {
						maxCount = counter[a];
						maxLabel = a;
					}

				if(maxCount > 0) {
					// and label all the pixels with this label
					for(auto& p : comp) {
						labels(p.first, p.second) = lightfields::SlicSuperpixels::Label(maxLabel);
						processed(p.first, p.second) = true;
					}
				}
				else
					unprocessed++;
			}
		}
	}
}

}
