#pragma once

#include <array>
#include <opencv2/opencv.hpp>

#include "grid.h"

namespace lightfields {

/// Implementation of the SLIC superpixels algorithm.
/// Achanta, Radhakrishna, et al. "SLIC superpixels compared to state-of-the-art superpixel methods." IEEE transactions
/// on pattern analysis and machine intelligence 34.11 (2012): 2274-2282. Each step of the algorithm is explicitly
/// exposed to allow for different wiring based on the use-case.
template <typename T>
struct SlicSuperpixels {
	/// Representation of one center of a superpixel
	struct Center {
		Center();
		Center(const cv::Mat& m, int r, int c);

		Center& operator+=(const Center& c);
		Center& operator/=(int div);

		int row, col;
		std::array<T, 3> color;
	};

	struct Label {
		Label(int lbl = 0, float metr = std::numeric_limits<float>::max()) noexcept : id(lbl), metric(metr) {
		}

		int id;
		float metric;
	};

	/// Implementation of the distance metric from the paper
	class Metric {
	  public:
		/// S is the superpixel distance; m is the spatial-to-color weight
		Metric(int S, float m);

		// implementation of eq. 3 of the paper
		float operator()(const lightfields::SlicSuperpixels<T>::Center& c,
		                 const cv::Mat& m,
		                 const int row,
		                 const int col) const;

		int S() const;

	  private:
		int m_S;
		float m_SS, m_mm;
	};

	/// Initialise the S (grid spacing) variable
	static int initS(int rows, int cols, int pixelCount);

	/// initialise the grid of superpixels
	static lightfields::Grid<lightfields::SlicSuperpixels<T>::Center> initPixels(const cv::Mat& in, int S);

	/// label all pixels based on the closest distance to centers
	static void label(const cv::Mat& in,
	                  lightfields::Grid<Label>& labels,
	                  const lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers,
	                  const Metric& metric);

	/// recompute the centres based on labels
	static void findCenters(const cv::Mat& in,
	                        const lightfields::Grid<Label>& labels,
	                        lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers);

	/// connect labels using connected components.
	/// First find the largest continuous labelled region for each label, then relabel and connect all the smaller ones
	/// to the nearest region. Results in one continuous region per superpixel, with all smaller ones merged into one of
	/// the large ones.
	static void connectedComponents(lightfields::Grid<Label>& labels,
	                                const lightfields::Grid<lightfields::SlicSuperpixels<T>::Center>& centers);
};

}  // namespace lightfields
