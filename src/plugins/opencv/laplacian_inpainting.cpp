#include "laplacian_inpainting.h"

#define EIGEN_STACK_ALLOCATION_LIMIT 0
#include <Eigen/Sparse>

#include <tbb/parallel_for.h>
#include <tbb/task_group.h>

namespace possumwood {
namespace opencv {

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
	Row& operator=(const Row&) = delete;

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

			m_triplets.push_back(Eigen::Triplet<double>(m_rowCount, row * m_cols + col, v.second));
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

static const cv::Mat kernel = (cv::Mat_<double>(3, 3) << 0.0, -1.0, 0.0, -1.0, 4.0, -1.0, 0.0, -1.0, 0.0);

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

float buildMatrices(const cv::Mat& image, const cv::Mat& mask, Eigen::SparseMatrix<double>& A, Eigen::VectorXd& b) {
	Triplets triplets(image.rows, image.cols);
	std::vector<double> values;

	std::size_t validCtr = 0, interpolatedCtr = 0;

	for(int y = 0; y < image.rows; ++y)
		for(int x = 0; x < image.cols; ++x) {
			Row row;

			// masked and/or edge
			if(mask.at<unsigned char>(y, x) > 128) {
				values.push_back(0.0f);

				// convolution
				for(int yi = 0; yi < kernel.rows; ++yi)
					for(int xi = 0; xi < kernel.cols; ++xi) {
						int ypos = y + yi - kernel.rows / 2;
						int xpos = x + xi - kernel.cols / 2;

						// handling of edges - "clip" (or "mirror", commented out for now)
						if(ypos < 0)
							// ypos = -ypos;
							ypos = 0;
						if(ypos >= image.rows)
							// ypos = (image.rows-1) - (ypos-image.rows);
							ypos = image.rows - 1;

						if(xpos < 0)
							// xpos = -xpos;
							xpos = 0;
						if(xpos >= image.cols)
							// xpos = (image.cols-1) - (xpos-image.cols);
							xpos = image.cols - 1;

						row.addValue(ypos, xpos, kernel.at<double>(yi, xi));
					}

				++interpolatedCtr;
			}

			// non-masked
			if(mask.at<unsigned char>(y, x) <= 128) {
				values.push_back(image.at<float>(y, x));
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
	for(std::size_t i = 0; i < values.size(); ++i)
		b[i] = values[i];

	return (float)validCtr / ((float)validCtr + (float)interpolatedCtr);
}

dependency_graph::State solve(const cv::Mat& input, const cv::Mat& mask, std::vector<float>& output) {
	assert(input.rows == mask.rows);
	assert(input.cols == mask.cols);
	assert(input.type() == CV_32FC1);
	assert(mask.type() == CV_8UC1);

	output = std::vector<float>(input.rows * input.cols, 0.0f);

	Eigen::SparseMatrix<double> A;
	Eigen::VectorXd b, tmp;

	dependency_graph::State state;

	const float ratio = buildMatrices(input, mask, A, b);

	if(ratio > 0.003) {
		const char* stage = "solver construction";

		Eigen::SparseLU<Eigen::SparseMatrix<double> /*, Eigen::NaturalOrdering<int>*/> chol(A);

		if(chol.info() == Eigen::Success) {
			stage = "analyze pattern";

			chol.analyzePattern(A);

			if(chol.info() == Eigen::Success) {
				stage = "factorize";

				chol.factorize(A);

				if(chol.info() == Eigen::Success) {
					stage = "solve";

					tmp = chol.solve(b);

					assert(tmp.size() == input.rows * input.cols);
					for(int i = 0; i < tmp.size(); ++i) {
						const int row = i / input.cols;
						const int col = i % input.cols;
						const int index = row * mask.cols + col;

						assert((std::size_t)index < output.size());
						output[index] = tmp[i];
					}
				}
			}
		}

		if(chol.info() == Eigen::NumericalIssue)
			state.addWarning("Decomposition failed - Eigen::NumericalIssue at stage " + std::string(stage));
		else if(chol.info() == Eigen::NoConvergence)
			state.addWarning("Decomposition failed - Eigen::NoConvergence at stage " + std::string(stage));
		else if(chol.info() == Eigen::InvalidInput)
			state.addWarning("Decomposition failed - Eigen::InvalidInput at stage " + std::string(stage));
		else if(chol.info() != Eigen::Success)
			state.addWarning("Decomposition failed - unknown error at stage " + std::string(stage));
	}
	else
		state.addError("Solve failed - not enough non-masked data in input matrix");

	return state;
}

}  // namespace

dependency_graph::State inpaint(const std::vector<cv::Mat>& inputs,
                                const std::vector<cv::Mat>& masks,
                                std::vector<cv::Mat>& result) {
	if(inputs.size() != masks.size() || inputs.empty() || masks.empty())
		throw std::runtime_error("Laplacian inpainting - number of inputs and masks has to match.");

	for(std::size_t i = 0; i < inputs.size(); ++i) {
		if(inputs[i].depth() != CV_32F)
			throw std::runtime_error("Laplacian inpainting - input image type has to be CV_32F.");
		if(masks[i].depth() != CV_8U || masks[i].type() != masks[0].type())
			throw std::runtime_error("Laplacian inpainting - mask image type has to be CV_8UC1 or CV_8UC3.");
		if(inputs[i].empty() || masks[i].empty())
			throw std::runtime_error("Laplacian inpainting - empty input image and/or mask.");
		if(inputs[i].size != masks[i].size || inputs[i].size != inputs[0].size)
			throw std::runtime_error("Laplacian inpainting - input and mask image size have to match.");
	}

	// split the inputs and masks per channel
	std::vector<cv::Mat> inputs_1ch, masks_1ch;

	for(std::size_t i = 0; i < inputs.size(); ++i) {
		std::vector<cv::Mat> i_tmp, m_tmp;

		cv::split(inputs[i], i_tmp);
		cv::split(masks[i], m_tmp);

		while(m_tmp.size() < i_tmp.size())
			m_tmp.push_back(m_tmp.back());

		inputs_1ch.insert(inputs_1ch.end(), i_tmp.begin(), i_tmp.end());
		masks_1ch.insert(masks_1ch.end(), m_tmp.begin(), m_tmp.end());
	}

	std::vector<std::vector<float>> x(inputs_1ch.size(), std::vector<float>(inputs[0].rows * inputs[0].cols, 0.0f));

	dependency_graph::State state;

	tbb::task_group tasks;
	std::mutex state_mutex;

	for(std::size_t channel = 0; channel < inputs_1ch.size(); ++channel) {
		tasks.run([channel, &inputs_1ch, &masks_1ch, &x, &state, &state_mutex]() {
			const dependency_graph::State currentState = solve(inputs_1ch[channel], masks_1ch[channel], x[channel]);

			std::lock_guard<std::mutex> guard(state_mutex);
			state.append(currentState);
		});
	}

	tasks.wait();

	result = std::vector<cv::Mat>();

	int channel = 0;
	for(auto& in : inputs) {
		cv::Mat tmp = cv::Mat::zeros(in.rows, in.cols, in.type());

		tbb::parallel_for(0, tmp.rows, [&](int yi) {
			for(int xi = 0; xi < tmp.cols; ++xi)
				for(int c = 0; c < in.channels(); ++c)
					tmp.ptr<float>(yi, xi)[c] = x[c + channel][yi * tmp.cols + xi];
		});

		channel += in.channels();
		result.push_back(tmp);
	}

	return state;
}

}  // namespace opencv
}  // namespace possumwood
