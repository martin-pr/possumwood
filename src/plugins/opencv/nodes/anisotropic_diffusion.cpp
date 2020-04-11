#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<float> a_coefficient, a_step;
dependency_graph::InAttr<unsigned> a_iterationLimit;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

/// creates an image representing the gradient magnitude, computed via Sobel filter
cv::Mat constant(const cv::Mat& in, float k) {
	assert(in.type() == CV_32FC1);
	cv::Mat mat = cv::Mat::zeros(in.rows, in.cols, CV_32FC1);

	static const float sobel[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

	tbb::parallel_for(tbb::blocked_range2d<int>(0, mat.rows, 0, mat.cols), [&](const tbb::blocked_range2d<int>& range) {
		for(int y=range.rows().begin(); y != range.rows().end(); ++y)
			for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
				float xsobel = 0.0f;
				float ysobel = 0.0f;

				for(int yi = -1; yi <= 1; ++yi)
					for(int xi = -1; xi <= 1; ++xi) {
						xsobel += sobel[xi+1][yi+1] * in.at<float>(std::min(std::max(0, y+yi), in.rows-1), std::min(std::max(0, x+xi), in.cols-1));
						ysobel += sobel[yi+1][xi+1] * in.at<float>(std::min(std::max(0, y+yi), in.rows-1), std::min(std::max(0, x+xi), in.cols-1));
					}

				mat.at<float>(y, x) = 1.0f / (1.0f + (xsobel*xsobel + ysobel*ysobel) / (k*k));
			}
	});

	return mat;
}

dependency_graph::State compute(dependency_graph::Values& data) {
	const cv::Mat& in = *data.get(a_in);

	if(in.type() != CV_32FC1)
		throw std::runtime_error("Input needs to be of type CV_32FC1.");

	cv::Mat mat = in.clone();
	cv::Mat source = mat.clone();

	for(unsigned it=0; it<data.get(a_iterationLimit); ++it) {
		cv::swap(source, mat);

		// compute the "constant" function
		const cv::Mat cf = constant(source, data.get(a_coefficient));

		tbb::parallel_for(tbb::blocked_range2d<int>(0, mat.rows, 0, mat.cols), [&](const tbb::blocked_range2d<int>& range) {
			for(int y=range.rows().begin(); y != range.rows().end(); ++y)
				for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
					float laplacian = 0.0f;
					float norm_x = 0.0f;
					float norm_y = 0.0f;

					float gradient_x = 0.0f;
					float gradient_y = 0.0f;
					float c_gradient_x = 0.0f;
					float c_gradient_y = 0.0f;

					float gradient = 0.0f;


					if(y > 0) {
						norm_y += 1.0f;

						laplacian += source.at<float>(y-1, x) - source.at<float>(y, x);
						gradient_y += source.at<float>(y, x) - source.at<float>(y-1, x);
						c_gradient_y += cf.at<float>(y, x) - cf.at<float>(y-1, x);

						gradient += (source.at<float>(y, x) - source.at<float>(y-1, x)) * (cf.at<float>(y, x) - cf.at<float>(y-1, x));
					}

					if(y < mat.rows-1) {
						norm_y += 1.0f;

						laplacian += source.at<float>(y+1, x) - source.at<float>(y, x);
						gradient_y += source.at<float>(y+1, x) - source.at<float>(y, x);
						c_gradient_y += cf.at<float>(y+1, x) - cf.at<float>(y, x);

						gradient += (source.at<float>(y+1, x) - source.at<float>(y, x)) * (cf.at<float>(y+1, x) - cf.at<float>(y, x));
					}

					if(x > 0) {
						norm_x += 1.0f;

						laplacian += source.at<float>(y, x-1) - source.at<float>(y, x);
						gradient_x += source.at<float>(y, x) - source.at<float>(y, x-1);
						c_gradient_x += cf.at<float>(y, x) - cf.at<float>(y, x-1);

						gradient += (source.at<float>(y, x) - source.at<float>(y, x-1)) * (cf.at<float>(y, x) - cf.at<float>(y, x-1));
					}

					if(x < mat.cols-1) {
						norm_x += 1.0f;

						laplacian += source.at<float>(y, x+1) - source.at<float>(y, x);
						gradient_x += source.at<float>(y, x+1) - source.at<float>(y, x);
						c_gradient_x += cf.at<float>(y, x+1) - cf.at<float>(y, x);

						gradient += (source.at<float>(y, x+1) - source.at<float>(y, x)) * (cf.at<float>(y, x+1) - cf.at<float>(y, x));
					}

					laplacian = laplacian / (norm_x + norm_y) * cf.at<float>(y, x);
					// laplacian = laplacian * cf.at<float>(y, x);

					// gradient = std::sqrt(std::pow(gradient_x / norm_x, 2) + std::pow(gradient_y / norm_y, 2))
					// 	* std::sqrt(std::pow(c_gradient_x / norm_x, 2) + std::pow(c_gradient_y / norm_y, 2));

					mat.at<float>(y, x) = source.at<float>(y, x) + (/*gradient  + */ laplacian ) * data.get(a_step);
					// mat.at<float>(y, x) = source.at<float>(y, x) + laplacian * data.get(a_step);

					// GRADIENT PART DOESN'T WORK - PRODUCES "STARS"


					// for(int yi = std::max(0, y-1); yi < std::min(source.rows, y+2); ++yi)
					// 	for(int xi = std::max(0, x-1); xi < std::min(source.cols, x+2); ++xi)
					// 		if(xi != x || yi != y) {
					// 			norm += 1.0f;
					// 			laplacian = laplacian + source.at<float>(yi, xi) - source.at<float>(y, x);
					// 		}

					// laplacian = laplacian * cf.at<float>(y, x);

					// mat.at<float>(y, x) = source.at<float>(y, x) + laplacian * data.get(a_step);
				}
		});
	}

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_coefficient, "coefficient", 0.1f);
	meta.addAttribute(a_step, "step", 0.01f);
	meta.addAttribute(a_iterationLimit, "iterations_limit", 10u);
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_coefficient, a_out);
	meta.addInfluence(a_step, a_out);
	meta.addInfluence(a_iterationLimit, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/anisotropic_diffusion", init);

}
