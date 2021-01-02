#include "slic_superpixels.h"

#include <tbb/blocked_range2d.h>
#include <tbb/parallel_for.h>

#include "bitfield.h"

namespace lightfields {

template <typename T>
SlicSuperpixels<T>::Center::Center() : m_row(0), m_col(0), m_color{{0, 0, 0}} {
}

template <typename T>
SlicSuperpixels<T>::Center::Center(const cv::Mat& m, int r, int c) : m_row(r), m_col(c) {
	assert(r >= 0 && r < m.rows);
	assert(c >= 0 && c < m.cols);

	auto ptr = m.ptr<T>(r, c);
	for(int a = 0; a < 3; ++a)
		m_color[a] = ptr[a];
}

template <typename T>
typename SlicSuperpixels<T>::Center& SlicSuperpixels<T>::Center::operator+=(const Center& c) {
	m_row += c.m_row;
	m_col += c.m_col;

	for(int a = 0; a < 3; ++a)
		m_color[a] += c.m_color[a];

	return *this;
}

template <typename T>
typename SlicSuperpixels<T>::Center& SlicSuperpixels<T>::Center::operator/=(int div) {
	m_row /= div;
	m_col /= div;

	for(int a = 0; a < 3; ++a)
		m_color[a] /= div;

	return *this;
}

template <typename T>
int SlicSuperpixels<T>::Center::row() const {
	return m_row;
}

template <typename T>
int SlicSuperpixels<T>::Center::col() const {
	return m_col;
}

template <typename T>
const std::array<float, 3>& SlicSuperpixels<T>::Center::color() const {
	return m_color;
}

////////////

template <typename T>
SlicSuperpixels<T>::Metric::Metric(int S, float m) : m_S(S), m_SS(S * S), m_mm(m * m) {
}

template <typename T>
float SlicSuperpixels<T>::Metric::operator()(const lightfields::SlicSuperpixels<T>::Center& c,
                                             const cv::Mat& m,
                                             const int row,
                                             const int col) const {
	float d_c = 0.0f;
	for(int a = 0; a < 3; ++a) {
		float elem = float(c.color()[a]) - float(m.ptr<T>(row, col)[a]);
		elem *= elem;

		d_c += elem;
	}

	const float d_s = float(c.row() - row) * float(c.row() - row) + float(c.col() - col) * float(c.col() - col);

	return std::sqrt(d_c + d_s / m_SS * m_mm);
}

template <typename T>
int SlicSuperpixels<T>::Metric::S() const {
	return m_S;
}

////////////

template <typename T>
int SlicSuperpixels<T>::initS(int rows, int cols, int pixelCount) {
	int result = std::sqrt((cols * rows) / pixelCount);
	if(result <= 0)
		throw std::runtime_error("Only positive pixel counts are allowed.");

	return result;
}

template <typename T>
lightfields::Grid<typename lightfields::SlicSuperpixels<T>::Center> SlicSuperpixels<T>::initPixels(const cv::Mat& in,
                                                                                                   int S) {
	const int rows = in.rows / S;
	const int cols = in.cols / S;

	lightfields::Grid<lightfields::SlicSuperpixels<T>::Center> pixels(rows, cols);

	for(int y = 0; y < rows; ++y)
		for(int x = 0; x < cols; ++x)
			pixels(y, x) =
			    lightfields::SlicSuperpixels<T>::Center(in, (in.rows * y) / rows + S / 2, (in.cols * x) / cols + S / 2);

	return pixels;
}

namespace {
template <typename T>
void labelRange(const tbb::blocked_range2d<int>& range,
                const cv::Mat& in,
                Grid<typename SlicSuperpixels<T>::Label>& labels,
                const Grid<typename SlicSuperpixels<T>::Center>& centers,
                const typename SlicSuperpixels<T>::Metric& metric) {
	for(std::size_t center_id = 0; center_id < centers.container().size(); ++center_id) {
		const typename lightfields::SlicSuperpixels<T>::Center& center = centers.container()[center_id];

		for(int y = std::max(range.rows().begin(), center.row() - metric.S());
		    y < std::min(range.rows().end(), center.row() + metric.S() + 1); ++y)
			for(int x = std::max(range.cols().begin(), center.col() - metric.S());
			    x < std::min(range.cols().end(), center.col() + metric.S() + 1); ++x) {
				const float dist = metric(center, in, y, x);
				typename SlicSuperpixels<T>::Label& current = labels(y, x);

				if(current.metric > dist)
					current = typename SlicSuperpixels<T>::Label(center_id, dist);
			}
	}
}
}  // namespace

template <typename T>
void SlicSuperpixels<T>::label(const cv::Mat& in,
                               lightfields::Grid<Label>& labels,
                               const lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers,
                               const Metric& metric) {
	assert(in.rows == (int)labels.rows() && in.cols == (int)labels.cols());

	// clear all labels
	tbb::parallel_for(0, int(labels.container().size()), [&](int a) { labels.container()[a] = Label(); });

	// // using the metric instance, label all pixels
	// tbb::parallel_for(tbb::blocked_range2d<int>(0, in.rows, 0, in.cols), [&](const tbb::blocked_range2d<int>& range)
	// { 	labelRange<T>(range, in, labels, centers, metric);
	// });

	labelRange<T>(tbb::blocked_range2d<int>(0, in.rows, 0, in.cols), in, labels, centers, metric);
}

template <typename T>
void SlicSuperpixels<T>::findCenters(const cv::Mat& in,
                                     const lightfields::Grid<Label>& labels,
                                     lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers) {
	std::vector<lightfields::SlicSuperpixels<T>::Center> sum(centers.container().size());
	std::vector<int> count(centers.container().size(), 0);

	for(int row = 0; row < in.rows; ++row)
		for(int col = 0; col < in.cols; ++col) {
			const int label = labels(row, col).id;
			assert(label >= 0 && label < (int)sum.size());

			sum[label] += lightfields::SlicSuperpixels<T>::Center(in, row, col);
			count[label]++;
		}

	for(std::size_t a = 0; a < sum.size(); ++a)
		if(count[a] > 0) {
			sum[a] /= count[a];
			centers.container()[a] = sum[a];
		}
}

template <typename T>
void SlicSuperpixels<T>::connectedComponents(
    lightfields::Grid<Label>& labels,
    const lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers) {
	// first, find all components for each label
	//  label - component - pixels (pair of ints)
	std::vector<std::vector<std::vector<std::pair<int, int>>>> components(centers.container().size());

	{
		lightfields::Grid<bool, lightfields::Bitfield> connected(labels.rows(), labels.cols());

		for(int y = 0; y < (int)labels.rows(); ++y)
			for(int x = 0; x < (int)labels.cols(); ++x) {
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

						if(!connected(current.first, current.second) &&
						   labels(current.first, current.second).id == currentLabel) {
							connected(current.first, current.second) = true;

							currentComponent.push_back(std::make_pair(current.first, current.second));

							if(current.first > 0)
								stack.push_back(std::make_pair(current.first - 1, current.second));
							if(current.first < (int)labels.rows() - 1)
								stack.push_back(std::make_pair(current.first + 1, current.second));
							if(current.second > 0)
								stack.push_back(std::make_pair(current.first, current.second - 1));
							if(current.second < (int)labels.cols() - 1)
								stack.push_back(std::make_pair(current.first, current.second + 1));
						}
					}
				}
			}
	}

	// the largest component is THE component for the label - let's remove it from the list of components to process
	lightfields::Grid<bool, lightfields::Bitfield> processed(labels.rows(), labels.cols());
	for(std::size_t a = 0; a < components.size(); ++a) {
		auto& label = components[a];

		if(!label.empty()) {
			std::size_t maxSize = 0;
			std::size_t maxIndex = 0;

			for(int i = 0; i < (int)label.size(); ++i)
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

		for(std::size_t label_id = 0; label_id < components.size(); ++label_id) {
			auto& label = components[label_id];

			for(auto& comp : label) {
				std::vector<unsigned> counter(components.size());

				for(auto& p : comp) {
					if(p.first > 0 && processed(p.first - 1, p.second))
						counter[labels(p.first - 1, p.second).id]++;
					if(p.first < (int)labels.rows() - 1 && processed(p.first + 1, p.second))
						counter[labels(p.first + 1, p.second).id]++;
					if(p.second > 0 && processed(p.first, p.second - 1))
						counter[labels(p.first, p.second - 1).id]++;
					if(p.second < (int)labels.cols() - 1 && processed(p.first, p.second + 1))
						counter[labels(p.first, p.second + 1).id]++;
				}

				// find the most prelevant resolved label (there might not be any!)
				std::size_t maxLabel = 0;
				unsigned maxCount = 0;

				for(std::size_t a = 0; a < counter.size(); ++a)
					if(maxCount < counter[a]) {
						maxCount = counter[a];
						maxLabel = a;
					}

				if(maxCount > 0) {
					// and label all the pixels with this label
					for(auto& p : comp) {
						labels(p.first, p.second) = lightfields::SlicSuperpixels<T>::Label(maxLabel);
						processed(p.first, p.second) = true;
					}
				}
				else
					unprocessed++;
			}
		}
	}
}

// explicit instantiation
template struct SlicSuperpixels<unsigned char>;
template struct SlicSuperpixels<float>;

}  // namespace lightfields
