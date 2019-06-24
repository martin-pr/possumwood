#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>

#include <opencv2/opencv.hpp>

#include <actions/traits.h>
#include <maths/io/vec3.h>

#include "frame.h"
#include "circles_grid.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Frame> a_inFrame;
dependency_graph::InAttr<possumwood::opencv::CirclesGrid> a_circlesGrid;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_outFrame;

dependency_graph::State compute(dependency_graph::Values& data) {
	cv::Mat frame = (*data.get(a_inFrame)).clone();

	const possumwood::opencv::CirclesGrid& grid = data.get(a_circlesGrid);

	cv::drawChessboardCorners(frame, grid.size(), *grid, grid.wasFound());

	data.set(a_outFrame, possumwood::opencv::Frame(frame));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_inFrame, "in_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_circlesGrid, "circles_grid", possumwood::opencv::CirclesGrid(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_outFrame, "out_frame", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_inFrame, a_outFrame);
	meta.addInfluence(a_circlesGrid, a_outFrame);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/calibration/draw_pattern", init);

}
