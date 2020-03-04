#include <possumwood_sdk/node_implementation.h>

#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>

#include <lightfields/graph.h>

#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::InAttr<float> a_constness;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_debug;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& sequence = data.get(a_in);

	if(sequence.size() < 2)
		throw std::runtime_error("At least two images required in the input sequence.");

	const int width = (**sequence.begin()).cols;
	const int height = (**sequence.begin()).rows;

	for(auto& f : sequence)
		if((*f).rows != height || (*f).cols != width)
			throw std::runtime_error("Consistent width and height is required in the input sequence.");

	for(auto& f : sequence)
		if((*f).type() != CV_8UC1)
			throw std::runtime_error("Only CV_8UC1 images accepted on input.");


	// temporary
	if(sequence.size() != 2)
		throw std::runtime_error("Only 2 images supported on input for the moment.");

	const cv::Mat& m1 = (**sequence.begin());
	const cv::Mat& m2 = (**(sequence.begin() + 1));

	lightfields::Graph graph(lightfields::V2i(m1.cols, m1.rows), std::max(0.0f, data.get(a_constness)));

	for(int row = 0; row < m1.rows; ++row)
		for(int col = 0; col < m1.cols; ++col) {
			const int v1 = m1.at<unsigned char>(row, col);
			const int v2 = m2.at<unsigned char>(row, col);
			graph.setValue(lightfields::V2i(col, row), v1, v2);
		}

	graph.solve();

	data.set(a_out, possumwood::opencv::Frame(graph.minCut()));

	possumwood::opencv::Sequence debug;
	debug.add(graph.sourceFlow());
	debug.add(graph.horizontalFlow());
	debug.add(graph.verticalFlow());
	debug.add(graph.sinkFlow());

	data.set(a_debug, debug);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in_sequence", possumwood::opencv::Sequence());
	meta.addAttribute(a_constness, "constness", 128.0f);
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_debug, "debug", possumwood::opencv::Sequence());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_constness, a_out);

	meta.addInfluence(a_in, a_debug);
	meta.addInfluence(a_constness, a_debug);

	meta.setCompute(compute);
}


possumwood::NodeImplementation s_impl("opencv/lightfields/st_graphcut", init);

}
