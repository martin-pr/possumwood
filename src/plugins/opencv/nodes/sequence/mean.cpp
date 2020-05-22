#include <possumwood_sdk/node_implementation.h>

#include <boost/noncopyable.hpp>

#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include "sequence.h"

namespace {

dependency_graph::InAttr<possumwood::opencv::Sequence> a_in;
dependency_graph::OutAttr<possumwood::opencv::Frame> a_out;

struct Reduce : public boost::noncopyable {
	Reduce(const possumwood::opencv::Sequence& seq) {
		for(auto& f : seq)
			mats.push_back(*f); // "shallow" copy
	}

	// splitting constructor - copies the input matrices but there is no need to initialise the accumulator
	// as it will get overwritten anyway
	Reduce(const Reduce& r, tbb::split) : mats(r.mats) {
	}

	/// converts a range of input images int double format, and sums them together
	void operator() (const tbb::blocked_range<std::size_t>& range) {
		assert(!range.empty());

		std::vector<cv::Mat> converted(range.size());
		for(std::size_t i=range.begin(); i != range.end(); ++i)
			mats[i].convertTo(converted[i - range.begin()], CV_MAKETYPE(CV_64F, mats[i].channels()));

		accum = converted.front();
		for(auto it = converted.begin()+1; it != converted.end(); ++it)
			accum += *it;
	}

	// combine two results together
	void join(const Reduce& r) {
		assert(accum.rows == r.accum.rows && accum.cols == r.accum.cols);
		accum += r.accum;
	}

	std::vector<cv::Mat> mats; // input matrices (shallow copy)
	cv::Mat accum; // resulting accumulator
};

dependency_graph::State compute(dependency_graph::Values& data) {
	const possumwood::opencv::Sequence& in = data.get(a_in);

	if(!in.isValid())
		throw std::runtime_error("Input sequence does not have consistent size or type.");

	cv::Mat out;

	if(in.size() > 1) {
		// parallelized reduction implementing the accumulation and data conversion on multiple threads
		Reduce r(in);
		tbb::parallel_reduce(tbb::blocked_range<std::size_t>(0, in.size()), r);

		// normalize the result of the accumulation, and convert back to the original type
		r.accum /= (double)in.size();
		r.accum.convertTo(out, in[0]->type());
	}

	else
		throw std::runtime_error("At least two frames in the input sequence required!");

	data.set(a_out, possumwood::opencv::Frame(out));

	return dependency_graph::State();
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_in, "in");
	meta.addAttribute(a_out, "out", possumwood::opencv::Frame(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_in, a_out);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("opencv/sequence/mean", init);

}
