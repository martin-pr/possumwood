#include <actions/traits.h>
#include <lightfields/edmonds_karp.h>
#include <lightfields/graph.h>
#include <lightfields/push_relabel.h>
#include <possumwood_sdk/node_implementation.h>
#include <tbb/parallel_for.h>

#include <opencv2/opencv.hpp>

#include "possumwood_sdk/datatypes/enum.h"
#include "sequence.h"
#include "tools.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in, a_contrast;
dependency_graph::InAttr<float> a_constness;
dependency_graph::InAttr<possumwood::Enum> a_mode;
dependency_graph::InAttr<float> a_inPower, a_contPower;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_debug;

enum Mode { kEdmondsKarp, kPushRelabel };

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& sequence = data.get(a_in);
	const possumwood::opencv::Sequence& contrast = data.get(a_contrast);

	if(sequence.empty() || sequence.hasOneElement())
		throw std::runtime_error("At least two images required in the input sequence.");
	if(!contrast.empty() && !possumwood::opencv::Sequence::hasMatchingKeys(contrast, sequence))
		throw std::runtime_error("Contrast sequence (if any) needs to be the same size as input sequence.");

	const int width = sequence.meta().cols;
	const int height = sequence.meta().rows;

	if(sequence.meta().type != CV_8UC1)
		throw std::runtime_error("Only CV_8UC1 sequences accepted on input.");

	if(contrast.meta().type != CV_8UC1 && contrast.meta().type != CV_8UC3)
		throw std::runtime_error("Only CV_8UC1 or CV_8UC3 images accepted as contrast input.");

	std::size_t count = 0;
	for(auto it = sequence.begin(); it != sequence.end(); ++it)
		++count;

	lightfields::Graph graph(lightfields::V2i(width, height), std::max(0.0f, data.get(a_constness)), count);

	// setting vertical data
	{
		std::vector<int> values(count, 0);

		for(int row = 0; row < height; ++row)
			for(int col = 0; col < width; ++col) {
				std::size_t ctr = 0;
				for(auto& m : sequence) {
					values[ctr] = 255 - m.second.at<unsigned char>(row, col);
					++ctr;
				}

				graph.setValue(lightfields::V2i(col, row), values, data.get(a_inPower));
			}
	}

	// setting horizontal data
	if(!contrast.empty()) {
		std::size_t a = 0;
		for(auto& l : contrast) {
			graph.setLayer(l.second, a, data.get(a_contPower));
			++a;
		}
	}

	if(data.get(a_mode).value() == "Edmonds-Karp")
		lightfields::EdmondsKarp::solve(graph);
	else
		lightfields::PushRelabel::solve(graph);

	data.set(a_out, possumwood::opencv::Frame(graph.minCut()));

	possumwood::opencv::Sequence debug;
	{
		std::size_t ctr = 0;
		for(auto m : graph.debug())
			debug(ctr++, 0) = std::move(m);
	}

	data.set(a_debug, debug);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "confidence/in", possumwood::opencv::Sequence());
	meta.addAttribute(a_inPower, "confidence/power", 1.0f);
	meta.addAttribute(a_contrast, "contrast/in", possumwood::opencv::Sequence());
	meta.addAttribute(a_constness, "contrast/constness", 128.0f);
	meta.addAttribute(a_contPower, "contrast/power", 1.0f);
	meta.addAttribute(a_mode, "mode", possumwood::Enum({"Edmonds-Karp", "Push-Relabel"}));
	meta.addAttribute(a_out, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_debug, "debug", possumwood::opencv::Sequence());

	meta.addInfluence(a_in, a_out);
	meta.addInfluence(a_inPower, a_out);
	meta.addInfluence(a_contrast, a_out);
	meta.addInfluence(a_mode, a_out);
	meta.addInfluence(a_constness, a_out);
	meta.addInfluence(a_contPower, a_out);

	meta.addInfluence(a_in, a_debug);
	meta.addInfluence(a_inPower, a_debug);
	meta.addInfluence(a_contrast, a_debug);
	meta.addInfluence(a_mode, a_debug);
	meta.addInfluence(a_constness, a_debug);
	meta.addInfluence(a_contPower, a_debug);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/lightfields/st_graphcut", init);

}  // namespace
