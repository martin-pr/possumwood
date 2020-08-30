#include <ImathMatrixAlgo.h>
#include <actions/io.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <unordered_map>

#include "datatypes/animation.h"
#include "tokenizer.h"

namespace {

class AmcTokenizer : public anim::Tokenizer {
  private:
	State start, filename_start, filename_separator, filename, comment, hash;

  public:
	AmcTokenizer(std::istream& in)
	    : anim::Tokenizer(in), start(this), filename_start(this), filename_separator(this), filename(this),
	      comment(this), hash(this) {
		// start parsing in "start" state
		start.setActive();

		start = [this](char c) {
			// don't read any space-like characters
			if(std::isspace(c)) {
				emit();
				reject();
			}

			// hashbangs and comments start with #
			else if(c == '#') {
				emit();
				reject();

				hash.setActive();
			}

			// anything else is an acceptable character
			else
				accept(c);
		};

		hash = [this](char c) {
			// decide if this is a hashbang (accepted as a keyword) or a comment
			if(c == '!') {
				accept("#!");
				filename_start.setActive();
			}
			else
				comment.setActive();
		};

		comment = [this](char c) {
			// endline character ends comments
			if(c == '\n')
				start.setActive();

			// reject everything
			reject();
		};

		filename_start = [this](char c) {
			// don't read any space-like characters
			if(std::isspace(c))
				filename_separator.setActive();

			// anything else is an acceptable character
			else
				accept(c);
		};

		filename_separator = [this](char c) {
			if(std::isspace(c)) {
				emit();
				reject();
			}
			else
				filename.setActive();
		};

		filename = [this](char c) {
			// endline character ends filename
			if(c == '\n') {
				emit();
				reject();

				start.setActive();
			}

			// else anything can be part of a filename, including spaces
			else
				accept(c);
		};
	}
};

////

Imath::V3f makeTranslation(float value, const std::string& key) {
	if(key == "tx")
		return Imath::V3f(value, 0, 0);
	else if(key == "ty")
		return Imath::V3f(0, value, 0);
	else if(key == "tz")
		return Imath::V3f(0, 0, value);
	else
		throw std::runtime_error("unknown transformation component " + key);
}

Imath::Quatf makeRotation(float value, const std::string& key) {
	Imath::Quatf q;
	if(key == "rx")
		q.setAxisAngle(Imath::V3f(1, 0, 0), value / 180.0f * M_PI);
	else if(key == "ry")
		q.setAxisAngle(Imath::V3f(0, 1, 0), value / 180.0f * M_PI);
	else if(key == "rz")
		q.setAxisAngle(Imath::V3f(0, 0, 1), value / 180.0f * M_PI);
	else
		throw std::runtime_error("unknown transformation component " + key);

	return q;
}

void readFrame(anim::Tokenizer& tokenizer, anim::Skeleton& skel,
               const std::unordered_map<std::string, unsigned>& jointIds) {
	const anim::Skeleton orig = skel;

	// reset all base transforms to identity first
	for(auto& j : skel)
		j.tr() = anim::Transform();

	auto it = jointIds.find(tokenizer.next().value);
	while(it != jointIds.end()) {
		// read values for the joint
		Imath::V3f tr(0, 0, 0);
		Imath::Quatf rot(1, 0, 0, 0);
		for(const std::string& dof : skel[it->second].attributes()["dof"].as<std::vector<std::string>>())
			if(dof[0] == 't')
				tr = makeTranslation(boost::lexical_cast<float>(tokenizer.next().value), dof) + tr;
			else
				rot = makeRotation(boost::lexical_cast<float>(tokenizer.next().value), dof) * rot;

		// assign the value
		skel[it->second].tr() = orig[it->second].tr() * anim::Transform(rot, tr);

		// and process next bone
		it = jointIds.find(tokenizer.next().value);
	}
}

// adapted from http://research.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/ASF-AMC.html
std::unique_ptr<anim::Animation> doLoad(const boost::filesystem::path& filename, const anim::Skeleton& skel) {
	// open the file and instantiate the tokenizer
	std::ifstream in(filename.string());
	AmcTokenizer tokenizer(in);

	// assuming the CMU database at 120 FPS
	std::unique_ptr<anim::Animation> result(new anim::Animation(120));

	unsigned frameNo = 1;

	// std::cout << "READING START " << filename << std::endl;
	// while(!tokenizer.eof())
	// 	std::cout << "  - " << tokenizer.next().value << std::endl;
	// std::cout << "READING END" << std::endl;

	// read the hashbang
	std::string asfFilename;
	tokenizer >> "#!OML:ASF" >> asfFilename;

	// build an index on top of the skeleton, to make bone lookup faster
	std::unordered_map<std::string, unsigned> jointIds;
	for(unsigned a = 0; a < skel.size(); ++a)
		jointIds[skel[a].name()] = a;
	assert(skel.size() == jointIds.size());

	tokenizer >> ":FULLY-SPECIFIED";
	tokenizer >> ":DEGREES";
	tokenizer.next();

	while(!tokenizer.eof()) {
		// read the frame number, which has to match the counter
		const unsigned currentFrame = boost::lexical_cast<unsigned>(tokenizer.current().value);
		if(currentFrame != frameNo)
			throw std::runtime_error("expecting frame #" + boost::lexical_cast<std::string>(frameNo) + " but found #" +
			                         tokenizer.current().value);

		// add a frame and read it
		anim::Skeleton frame = skel;
		readFrame(tokenizer, frame, jointIds);
		result->addFrame(frame);

		++frameNo;
	}

	return result;
}

//////////

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::InAttr<anim::Skeleton> a_skel;
dependency_graph::OutAttr<anim::Animation> a_anim;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);
	const anim::Skeleton skel = data.get(a_skel);

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename()) && !skel.empty()) {
		std::unique_ptr<anim::Animation> ptr(doLoad(filename.filename(), skel).release());
		data.set(a_anim, *ptr);
	}
	else {
		data.set(a_anim, anim::Animation());
		out.addError("Cannot load filename " + filename.filename().string());
	}

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename",
	                  possumwood::Filename({
	                      "AMC files (*.amc)",
	                  }));
	meta.addAttribute(a_skel, "skeleton", anim::Skeleton(), possumwood::AttrFlags::kVertical);
	meta.addAttribute(a_anim, "animation", anim::Animation(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_anim);
	meta.addInfluence(a_skel, a_anim);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/loaders/amc", init);

}  // namespace
