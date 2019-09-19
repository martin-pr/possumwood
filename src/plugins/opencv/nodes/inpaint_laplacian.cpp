#include <possumwood_sdk/node_implementation.h>

#define EIGEN_STACK_ALLOCATION_LIMIT 0

#include <mutex>

#include <opencv2/opencv.hpp>

#include <Eigen/Sparse>

#include <tbb/task_group.h>

#include <actions/traits.h>

#include "frame.h"

namespace {

class Triplets;

class Row {
	public:
		Row() = default;

		void addValue(int64_t row, int64_t col, double value) {
			if(value != 0.0)
				m_values[(row << 32) + col] += value;
		}

	private:
		Row(const Row&) = delete;
		Row& operator = (const Row&) = delete;

		std::map<int64_t, double> m_values;

	friend class Triplets;
};

class Triplets {
	public:
		Triplets(int rows, int cols) : m_rowCount(0), m_rows(rows), m_cols(cols) {
		}

		void addRow(const Row& r) {
			for(auto& v : r.m_values) {
				int32_t row = v.first >> 32;
				int32_t col = v.first & 0xffffffff;

				assert(row < m_rows);
				assert(col < m_cols);

				m_triplets.push_back(Eigen::Triplet<double>(m_rowCount, row*m_cols + col, v.second));
			}

			++m_rowCount;
		}

		std::size_t rows() const {
			return m_rowCount;
		}

		const std::vector<Eigen::Triplet<double>>& triplets() const {
			return m_triplets;
		}

	private:
		std::vector<Eigen::Triplet<double>> m_triplets;

		int m_rowCount, m_rows, m_cols;

		friend class Row;
};


static const cv::Mat kernel = (cv::Mat_<double>(3,3) <<
	 0.0, -1.0,  0.0,
	-1.0,  4.0, -1.0,
	 0.0, -1.0,  0.0
);

// static const cv::Mat kernel = (cv::Mat_<double>(3,3) <<
// 	-1.0, -1.0, -1.0,
// 	-1.0,  8.0, -1.0,
// 	-1.0, -1.0, -1.0
// );

// static const cv::Mat kernel = (cv::Mat_<double>(3,3) <<
// 	-1.0, -2.0, -1.0,
// 	-2.0, 12.0, -2.0,
// 	-1.0, -2.0, -1.0
// );

// static const cv::Mat kernel = (cv::Mat_<double>(5,5) <<
// 	 0.0,  0.0,  1.0,  0.0,  0.0,
// 	 0.0,  2.0, -8.0,  2.0,  0.0,
// 	 1.0, -8.0, 20.0, -8.0,  1.0,
// 	 0.0,  2.0, -8.0,  2.0,  0.0,
// 	 0.0,  0.0,  1.0,  0.0,  0.0
// );

float buildMatrices(const cv::Mat& image, const cv::Mat& mask, Eigen::SparseMatrix<double>& A, Eigen::VectorXd& b, const cv::Rect2i& roi) {
	Triplets triplets(roi.height, roi.width);
	std::vector<double> values;

	std::size_t validCtr = 0, interpolatedCtr = 0;

	for(int y=roi.y; y<roi.y+roi.height; ++y)
		for(int x=roi.x; x<roi.x+roi.width; ++x) {
			Row row;

			// masked and/or edge
			if(mask.at<unsigned char>(y, x) > 128) {
				values.push_back(0.0f);

				// convolution
				for(int yi=0; yi<kernel.rows; ++yi)
					for(int xi=0; xi<kernel.cols; ++xi) {
						int ypos = y + yi - kernel.rows/2;
						int xpos = x + xi - kernel.cols/2;

						// handling of edges - "clip" (or "mirror", commented out for now)
						if(ypos < roi.y)
							// ypos = -ypos;
							ypos = roi.y;
						if(ypos >= roi.y + roi.height)
							// ypos = (image.rows-1) - (ypos-image.rows);
							ypos = roi.y + roi.height - 1;

						if(xpos < roi.x)
							// xpos = -xpos;
							xpos = roi.x;
						if(xpos >= roi.x + roi.width)
							// xpos = (image.cols-1) - (xpos-image.cols);
							xpos = roi.x + roi.width - 1;

						row.addValue(ypos - roi.y, xpos - roi.x, kernel.at<double>(yi, xi));
					}

				++interpolatedCtr;
			}

			// non-masked
			if(mask.at<unsigned char>(y, x) <= 128) {
				values.push_back(image.at<float>(y, x));
				row.addValue(y-roi.y, x-roi.x, 1);

				++validCtr;
			}

			triplets.addRow(row);
		}

	// initialise the sparse matrix
	A = Eigen::SparseMatrix<double>(triplets.rows(), roi.height * roi.width);
	A.setFromTriplets(triplets.triplets().begin(), triplets.triplets().end());

	// and the "b" vector
	assert(values.size() == triplets.rows());
	b = Eigen::VectorXd(values.size());
	for(std::size_t i=0; i<values.size(); ++i)
		b[i] = values[i];

	return (float)validCtr / ((float)validCtr + (float)interpolatedCtr);
}


dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame, a_inMask;
dependency_graph::InAttr<unsigned> a_mosaic;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State state;

	const cv::Mat& input = *data.get(a_inFrame);
	const cv::Mat& mask = *data.get(a_inMask);
	const unsigned mosaic = data.get(a_mosaic);

	if(input.depth() != CV_32F)
		throw std::runtime_error("Laplacian inpainting - input image type has to be CV_32F.");
	if(mask.type() != CV_8UC1 && mask.type() != CV_8UC3)
		throw std::runtime_error("Laplacian inpainting - mask image type has to be CV_8UC1 or CV_8UC3.");
	if(input.empty() || mask.empty())
		throw std::runtime_error("Laplacian inpainting - empty input image and/or mask.");
	if(input.size != mask.size)
		throw std::runtime_error("Laplacian inpainting - input and mask image size have to match.");
	if(input.cols % mosaic != 0 || input.rows % mosaic != 0)
		throw std::runtime_error("Laplacian inpainting - image size is not divisible by mosaic count - invalid mosaic?.");

	std::vector<std::vector<float>> x(input.channels(), std::vector<float>(input.rows * input.cols, 0.0f));

	tbb::task_group tasks;
	std::mutex solve_mutex;

	// split the inputs and masks per channel
	std::vector<cv::Mat> inputs, masks;
	cv::split(input, inputs);
	cv::split(mask, masks);

	assert((int)inputs.size() == input.channels());
	assert((int)masks.size() == mask.channels());

	const unsigned mosaic_rows = input.rows / mosaic;
	const unsigned mosaic_cols = input.cols / mosaic;

	for(unsigned yi=0;yi<mosaic; ++yi) {
		for(unsigned xi=0;xi<mosaic; ++xi) {
			cv::Rect2i roi;
			roi.y = yi * mosaic_rows;
			roi.x = xi * mosaic_cols;
			roi.height = mosaic_rows;
			roi.width = mosaic_cols;

			for(int channel=0; channel<input.channels(); ++channel) {

				tasks.run([channel, &inputs, &masks, &x, &state, &solve_mutex, roi]() {
					cv::Mat inTile = inputs[channel];

					cv::Mat inMask;
					if(masks.size() == 1)
						inMask = masks[0];
					else
						inMask = masks[channel];

					Eigen::SparseMatrix<double> A;
					Eigen::VectorXd b, tmp;

					const float ratio = buildMatrices(inTile, inMask, A, b, roi);

					if(ratio > 0.003) {
						const char* stage = "solver construction";

						Eigen::SparseLU<Eigen::SparseMatrix<double> /*, Eigen::NaturalOrdering<int>*/ > chol(A);

						if(chol.info() == Eigen::Success) {
							stage = "analyze pattern";

							chol.analyzePattern(A);

							if(chol.info() == Eigen::Success) {
								stage = "factorize";

								chol.factorize(A);

								if(chol.info() == Eigen::Success) {
									stage = "solve";

									tmp = chol.solve(b);

									assert(tmp.size() == roi.height * roi.width);
									for(int i=0;i<tmp.size();++i) {
										const int row = i / roi.width;
										const int col = i % roi.width;
										const int index = (row + roi.y) * masks[0].cols + col + roi.x;

										assert((std::size_t)index < x[channel].size());

										x[channel][index] = tmp[i];
									}
								}
							}
						}

						std::lock_guard<std::mutex> guard(solve_mutex);

						if(chol.info() == Eigen::NumericalIssue)
							state.addWarning("Decomposition failed - Eigen::NumericalIssue at stage " + std::string(stage));
						else if(chol.info() == Eigen::NoConvergence)
							state.addWarning("Decomposition failed - Eigen::NoConvergence at stage " + std::string(stage));
						else if(chol.info() == Eigen::InvalidInput)
							state.addWarning("Decomposition failed - Eigen::InvalidInput at stage " + std::string(stage));
						else if(chol.info() != Eigen::Success)
							state.addWarning("Decomposition failed - unknown error at stage " + std::string(stage));
					}
				});
			}
		}
	}

	tasks.wait();

	cv::Mat result = input.clone();
	for(int yi=0;yi<result.rows;++yi)
		for(int xi=0;xi<result.cols;++xi)
			for(int c=0;c<input.channels();++c)
				result.ptr<float>(yi, xi)[c] = x[c][yi*result.cols + xi];

	data.set(a_outFrame, possumwood::opencv::Frame(result));

	return state;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_inMask, "mask", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_mosaic, "mosaic", 1u);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_inMask, a_outFrame);
	meta.addInfluence(a_mosaic, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/inpaint_laplacian", init);

}
