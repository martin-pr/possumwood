#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include "frame.h"
#include "lightfield_samples.h"
#include "tools.h"
#include "bspline.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<possumwood::opencv::LightfieldSamples> a_samples;
dependency_graph::InAttr<unsigned> a_width, a_height;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::LightfieldSamples& samples = data.get(a_samples);

	if((*data.get(a_in)).type() != CV_32FC1 && (*data.get(a_in)).type() != CV_32FC3)
		throw std::runtime_error("Only 32-bit single-float or 32-bit 3 channel float format supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	const cv::Mat& input = *data.get(a_in);

	const unsigned width = data.get(a_width);
	const unsigned height = data.get(a_height);

	// TODO: for parallelization to work reliably, we need to use integer atomics here, unfortunately

	// debug output of a small window into a text file - useful for gnuplot
	// {
	// 	std::vector<cv::Vec2d> points;
	// 	for(auto& s : samples) 
	// 		if(s.color == 0) {
	// 			cv::Vec2d pt = s.target;

	// 			if(pt[0] > 0.48 && pt[0] < 0.52 && pt[1] > 0.48 && pt[1] < 0.52)
	// 				points.push_back(pt);
	// 		}
		
	// 	std::ofstream data("data.txt");
	// 	for(auto& p : points)
	// 		data << std::setprecision(15) << p[0] << "\t" << std::setprecision(15) << p[1] << std::endl;
	// }

	// lets make a single B-spline, to test this
	possumwood::opencv::BSpline bspline[3] = {
		possumwood::opencv::BSpline(128),
		possumwood::opencv::BSpline(128),
		possumwood::opencv::BSpline(128)
	};
	for(auto s : samples)
		bspline[s.color].addSample(s.target[0], s.target[1], input.at<float>(s.source[1], s.source[0]));


	cv::Mat mat = cv::Mat::zeros(height, width, CV_32FC3);
	tbb::parallel_for(0, mat.rows, [&](int y) {
		for(int x=0;x<mat.cols;++x)
			for(int a=0;a<3;++a)
				mat.ptr<float>(y,x)[a] = bspline[a].sample(
					(float)x / (float)(mat.cols-1),
					(float)y / (float)(mat.rows-1)
				);
	});

	// debug printout of the bspline controls
	// for(unsigned a=0;a<3;++a)
	// 	std::cout << bspline[a] << std::endl;

	data.set(a_out, possumwood::opencv::Frame(mat));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_samples, "samples", possumwood::opencv::LightfieldSamples(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_width, "size/width", 300u);
	meta.addAttribute(a_height, "size/height", 300u);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_samples, a_out);
	meta.addInfluence(a_width, a_out);
	meta.addInfluence(a_height, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/integrate_bezier", init);

}
