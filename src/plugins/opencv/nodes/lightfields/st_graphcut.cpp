#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/graph.h>

#include "frame.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_in;
dependency_graph::InAttr<float> a_constness;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat values = (*data.get(a_in)).clone();

	if(values.type() != CV_8UC1)
		throw std::runtime_error("Only 8-bit unsigned char supported on input, " + possumwood::opencv::type2str((*data.get(a_in)).type()) + " found instead!");

	lightfields::Graph graph(Imath::V2i(values.cols, values.rows), std::max(0.0f, data.get(a_constness)));

	for(int row = 0; row < values.rows; ++row)
		for(int col = 0; col < values.cols; ++col) {
			const float val = (float)values.at<unsigned char>(row, col) / 255.0f;
			graph.setValue(Imath::V2i(col, row), 1.0f-val, val);
		}

	graph.solve();

	auto sources = graph.sourceGraph();
	// auto sinks = graph.sinkGraph();

	for(int row = 0; row < values.rows; ++row)
		for(int col = 0; col < values.cols; ++col)
			if(sources.find(Imath::V2i(col, row)) != sources.end())
				values.at<unsigned char>(row, col) = 0;
			else //if(sinks.find(Imath::V2i(col, row)) != sinks.end())
				values.at<unsigned char>(row, col) = 255;
			// else
			// 	values.at<unsigned char>(row, col) = 128;

	data.set(a_out, possumwood::opencv::Frame(values));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_constness, "constness", 128.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_constness, a_out);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/st_graphcut", init);

}
