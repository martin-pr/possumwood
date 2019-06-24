#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <possumwood_sdk/datatypes/enum.h>
#include <actions/traits.h>

#include "frame.h"
#include "circles_grid.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_frame;
dependency_graph::InAttr<possumwood::Enum> a_flags;
dependency_graph::InAttr<bool> a_clustering;
dependency_graph::InAttr<unsigned> a_sizeWidth, a_sizeHeight;
dependency_graph::OutAttr<possumwood::opencv::CirclesGrid> a_circlesGrid;

int flagsToEnum(const std::string& flags) {
	if(flags == "Symmetric grid")
		return cv::CALIB_CB_SYMMETRIC_GRID;
	else if(flags == "Asymmetric grid")
		return cv::CALIB_CB_ASYMMETRIC_GRID;

	throw std::runtime_error("Unknown value " + flags);
}

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat result;

	int flags = flagsToEnum(data.get(a_flags).value());
	if(data.get(a_clustering))
		flags |= cv::CALIB_CB_CLUSTERING;

	const bool success = cv::findCirclesGrid(*data.get(a_frame), 
		cv::Size(data.get(a_sizeWidth), data.get(a_sizeHeight)),
		result,	flags
	);

	dependency_graph::State status;
	if(!success)
		status.addWarning("No circle grid found in the input image!");

	const possumwood::opencv::CirclesGrid grid(result,
		cv::Size(data.get(a_sizeWidth), data.get(a_sizeHeight)), success);
	data.set(a_circlesGrid, grid);
	
	return status;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_frame, "frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_flags, "flags", 
		possumwood::Enum({"Symmetric grid", "Asymmetric grid"}));
	meta.addAttribute(a_clustering, "clustering");
	meta.addAttribute(a_sizeWidth, "pattern_size/width", 4u);
	meta.addAttribute(a_sizeHeight, "pattern_size/height", 11u);
	meta.addAttribute(a_circlesGrid, "circles_grid", possumwood::opencv::CirclesGrid(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_frame, a_circlesGrid);
	meta.addInfluence(a_flags, a_circlesGrid);
	meta.addInfluence(a_clustering, a_circlesGrid);
	meta.addInfluence(a_sizeWidth, a_circlesGrid);
	meta.addInfluence(a_sizeHeight, a_circlesGrid);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/find_circles", init);

}
