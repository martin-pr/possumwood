#include <actions/traits.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <opencv2/opencv.hpp>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<unsigned> a_start, a_count;
dependency_graph::OutAttr<possumwood::opencv::Sequence> a_sequence;

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::Filename filename = data.get(a_filename);

	// open the capture device
	cv::VideoCapture cap(filename.filename().string());
// seek to the requested frame
#if CV_MAJOR_VERSION > 2
	cap.set(cv::CAP_PROP_POS_FRAMES, data.get(a_start));
#else
	cap.set(CV_CAP_PROP_POS_FRAMES, data.get(a_start));
#endif

	// and feed the read frames into the Sequence datastructure
	possumwood::opencv::Sequence result;

	for(std::size_t f = 0; f < data.get(a_count); ++f) {
		cv::Mat frame;

		cap >> frame;

		result(f, 0) = std::move(frame);
	}

	data.set(a_sequence, result);

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename",
	                  possumwood::Filename({
	                      "MP4 files (*.mp4)",
	                  }));
	meta.addAttribute(a_start, "frame_range/start");
	meta.addAttribute(a_count, "frame_range/count");
	meta.addAttribute(a_sequence, "sequence");

	meta.addInfluence(a_filename, a_sequence);
	meta.addInfluence(a_start, a_sequence);
	meta.addInfluence(a_count, a_sequence);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/capture/video_sequence", init);

}  // namespace
