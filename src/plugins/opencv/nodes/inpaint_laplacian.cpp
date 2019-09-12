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

float buildMatrices(const cv::Mat& image, const cv::Mat& mask, Eigen::SparseMatrix<double>& A, Eigen::VectorXd& b, int channel) {
	Triplets triplets(image.rows, image.cols);
	std::vector<double> values;

	std::size_t validCtr = 0, interpolatedCtr = 0;

	for(int y=0;y<image.rows;++y)
		for(int x=0;x<image.cols;++x) {
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
						if(ypos < 0)
							// ypos = -ypos;
							ypos = 0;
						if(ypos >= image.rows)
							// ypos = (image.rows-1) - (ypos-image.rows);
							ypos = image.rows-1;

						if(xpos < 0)
							// xpos = -xpos;
							xpos = 0;
						if(xpos >= image.cols)
							// xpos = (image.cols-1) - (xpos-image.cols);
							xpos = image.cols-1;

						row.addValue(ypos, xpos, kernel.at<double>(yi, xi));
					}

				++interpolatedCtr;
			}

			// non-masked
			if(mask.at<unsigned char>(y, x) <= 128) {
				values.push_back(image.ptr<float>(y, x)[channel]);
				row.addValue(y, x, 1);

				++validCtr;
			}

			triplets.addRow(row);
		}

	// initialise the sparse matrix
	A = Eigen::SparseMatrix<double>(triplets.rows(), image.rows * image.cols);
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

	const cv::Mat& image = *data.get(a_inFrame);
	const cv::Mat& mask = *data.get(a_inMask);
	const unsigned mosaic = data.get(a_mosaic);

	if(image.depth() != CV_32F)
		throw std::runtime_error("Laplacian inpainting - input image type has to be CV_32F.");
	if(mask.type() != CV_8UC1)
		throw std::runtime_error("Laplacian inpainting - mask image type has to be CV_8UC1.");
	if(image.empty() || mask.empty())
		throw std::runtime_error("Laplacian inpainting - empty input image and/or mask.");
	if(image.size != mask.size)
		throw std::runtime_error("Laplacian inpainting - input and mask image size have to match.");

	std::vector<std::vector<float>> x(image.channels(), std::vector<float>(image.rows * image.cols, 0.0f));

	tbb::task_group tasks;
	std::mutex solve_mutex;

	for(unsigned a=0;a<mosaic; ++a) {
		for(unsigned b=0;b<mosaic; ++b) {
			for(int channel=0; channel<image.channels(); ++channel) {
				cv::Rect2i roi;
				roi.y = (a * image.rows) / mosaic;
				roi.x = (b * image.cols) / mosaic;
				roi.height = ((a+1) * image.rows) / mosaic - roi.y;
				roi.width = ((b+1) * image.cols) / mosaic - roi.x;

				tasks.run([channel, &image, &mask, &x, &state, &solve_mutex, roi]() {
					cv::Mat inTile(image, roi);
					cv::Mat inMask(mask, roi);

					Eigen::SparseMatrix<double> A;
					Eigen::VectorXd b, tmp;

					const float ratio = buildMatrices(inTile, inMask, A, b, channel);

					if(ratio > 0.003) {
						const char* stage = "solver construction";

						Eigen::SparseLU<Eigen::SparseMatrix<double>, Eigen::NaturalOrdering<int>> chol(A);

						if(chol.info() == Eigen::Success) {
							stage = "analyze pattern";

							chol.analyzePattern(A);

							if(chol.info() == Eigen::Success) {
								stage = "factorize";

								chol.factorize(A);

								if(chol.info() == Eigen::Success) {
									stage = "solve";

									tmp = chol.solve(b);

									assert(tmp.size() == inTile.rows * inTile.cols);
									for(int i=0;i<tmp.size();++i) {
										const int row = i / roi.width;
										const int col = i % roi.width;
										const int index = (row + roi.y) * image.cols + col + roi.x;

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

	cv::Mat result = image.clone();
	for(int yi=0;yi<result.rows;++yi)
		for(int xi=0;xi<result.cols;++xi)
			for(int c=0;c<image.channels();++c)
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
