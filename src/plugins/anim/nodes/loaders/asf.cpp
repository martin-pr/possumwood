#include <OpenEXR/ImathVec.h>
#include <actions/io.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <possumwood_sdk/node_implementation.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <cctype>
#include <fstream>
#include <iostream>

#include "datatypes/attributes.h"
#include "datatypes/skeleton.h"
#include "datatypes/transform.h"
#include "tokenizer.h"

using std::cout;
using std::endl;

namespace {

class AsfTokenizer : public anim::Tokenizer {
  private:
	State start, keyword, comment;

  public:
	AsfTokenizer(std::istream& in) : Tokenizer(in), start(this), keyword(this), comment(this) {
		// start parsing in "start" state
		start.setActive();

		start = [this](char c) {
			// don't read any space-like characters
			if(std::isspace(c))
				reject();

			// comments start with #
			else if(c == '#')
				comment.setActive();

			// brackets are handled separately (no need for a state)
			else if(c == '(' || c == ')') {
				accept(c);
				emit();
			}

			// everything else is a "keyword"
			else
				keyword.setActive();
		};

		comment = [this](char c) {
			// endline character ends comment
			if(c == '\n')
				start.setActive();

			// reject everything
			reject();
		};

		keyword = [this](char c) {
			// space or bracket characters end a keyword
			if(std::isspace(c) || c == '(' || c == ')') {
				emit();
				start.setActive();
			}
			// otherwise just parse another character of the keyword
			else
				accept(c);
		};

		// read the first keyword into current
		next();
	}
};

///////////////////

struct Joint {
	std::string name;
	Imath::V3f position;
	anim::Transform axis;
	int parent = -1;
	anim::Attributes attrs;
};

void readUnits(anim::Tokenizer& tokenizer) {
	tokenizer.next();

	// for now just check that the file is ok, but don't try to do
	// anything with the unit values (this might change)
	while(true) {
		if(tokenizer.current().value == "mass")
			tokenizer.next();
		else if(tokenizer.current().value == "length")
			tokenizer.next();
		else if(tokenizer.current().value == "angle")
			tokenizer.next();
		else
			break;

		// read next token
		tokenizer.next();
	}
}

void readDocumentation(anim::Tokenizer& tokenizer) {
	tokenizer.next();

	// skip until next keyword
	while(!tokenizer.current().value.empty() && tokenizer.current().value[0] != ':')
		tokenizer.next();
}

Imath::V3f readVec3(anim::Tokenizer& tokenizer) {
	// has to be written on separate lines to prevent the compiler from reordering the next() calls
	const float x = boost::lexical_cast<float>(tokenizer.next().value);
	const float y = boost::lexical_cast<float>(tokenizer.next().value);
	const float z = boost::lexical_cast<float>(tokenizer.next().value);

	return Imath::V3f(x, y, z);
}

// makes an XYZ rotation transform
anim::Transform makeAxisTransform(const Imath::V3f& v) {
	Imath::Quatf qx;
	qx.setAxisAngle(Imath::V3f(1, 0, 0), v.x / 180.0f * M_PI);

	Imath::Quatf qy;
	qy.setAxisAngle(Imath::V3f(0, 1, 0), v.y / 180.0f * M_PI);

	Imath::Quatf qz;
	qz.setAxisAngle(Imath::V3f(0, 0, 1), v.z / 180.0f * M_PI);

	return anim::Transform(qz * qy * qx);
}

void readRoot(anim::Tokenizer& tokenizer, std::vector<Joint>& bones) {
	if(!bones.empty())
		throw std::runtime_error(":root tag is not placed before bonedata tag");

	// for now, just read whatever is required and skip it
	tokenizer.next();

	// the resulting joint value
	Joint root;
	root.name = "root";

	// initialise the DOF field in joint attributes (might end up empty)
	std::vector<std::string>& dofs = root.attrs["dof"].as<std::vector<std::string>>();

	while(!tokenizer.current().value.empty() && tokenizer.current().value[0] != ':') {
		if(tokenizer.current().value == "order") {
			static const boost::regex value("(T|R)(X|Y|Z)");
			while(boost::regex_match(tokenizer.next().value, value)) {
				std::string dof = tokenizer.current().value;
				boost::algorithm::to_lower(dof);

				dofs.push_back(dof);
			}
		}

		else if(tokenizer.current().value == "axis") {
			if(tokenizer.next().value != "XYZ")
				throw("unknown axis value in :root - " + tokenizer.current().value);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "position") {
			root.position = readVec3(tokenizer);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "orientation") {
			root.axis = readVec3(tokenizer);
			tokenizer.next();
		}

		else
			throw std::runtime_error("unknown value in :root section - " + tokenizer.current().value);
	}

	// assuming we've read the root bone data - make the bone
	//   ignoring orientation for now
	bones.push_back(root);
}

std::pair<Joint, unsigned> readJoint(anim::Tokenizer& tokenizer) {
	// the result - Joint and its ID
	std::pair<Joint, unsigned> result;
	// initialise the DOF field in joint attributes (might end up empty)
	std::vector<std::string>& dofs = result.first.attrs["dof"].as<std::vector<std::string>>();

	// skip the begin token
	tokenizer.next();

	while(true) {
		if(tokenizer.current().value == "end")
			break;

		else if(tokenizer.current().value == "id") {
			result.second = boost::lexical_cast<unsigned>(tokenizer.next().value);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "name") {
			result.first.name = tokenizer.next().value;
			tokenizer.next();
		}

		else if(tokenizer.current().value == "direction") {
			result.first.position = readVec3(tokenizer);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "length") {
			result.first.position *= boost::lexical_cast<float>(tokenizer.next().value);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "axis") {
			result.first.axis = makeAxisTransform(readVec3(tokenizer));
			if(tokenizer.next().value != "XYZ")
				throw std::runtime_error("invalid axis rotation order " + tokenizer.next().value);
			tokenizer.next();
		}

		else if(tokenizer.current().value == "dof") {
			static const boost::regex value("r(x|y|z)");
			while(boost::regex_match(tokenizer.next().value, value))
				dofs.push_back(tokenizer.current().value);
		}

		else if(tokenizer.current().value == "limits") {
			tokenizer.next();

			// read all expressions in brackets
			while(tokenizer.current().value == "(") {
				tokenizer.next();

				// two numbers
				tokenizer.next();
				tokenizer.next();

				// closing bracked
				tokenizer.next();
			}
		}

		else
			throw std::runtime_error("unknown value in joint section - " + tokenizer.current().value);
	}

	// skip the end token
	tokenizer.next();

	return result;
}

void readBoneData(anim::Tokenizer& tokenizer, std::vector<Joint>& bones) {
	tokenizer.next();

	while(tokenizer.current().value == "begin") {
		std::pair<Joint, unsigned> j = readJoint(tokenizer);
		if(j.second != bones.size())
			throw std::runtime_error("invalid index for bone " + j.first.name);
		bones.push_back(j.first);
	}
}

unsigned findIndex(const std::vector<Joint>& bones, const std::string& name) {
	int result = -1;
	for(unsigned a = 0; a < bones.size(); ++a)
		if(bones[a].name == name)
			result = a;

	if(result < 0)
		throw std::runtime_error(":hierarchy refers to bone " + name + " which was not defined in :bonedata section");

	return result;
}

void readHierarchy(anim::Tokenizer& tokenizer, std::vector<Joint>& bones) {
	if(tokenizer.next().value != "begin")
		throw std::runtime_error(":hierarchy section does not have begin tag");
	tokenizer.next();

	while(tokenizer.current().value != "end") {
		const unsigned line = tokenizer.current().line;

		const unsigned parent = findIndex(bones, tokenizer.current().value);
		tokenizer.next();

		while(tokenizer.current().line == line) {
			bones[findIndex(bones, tokenizer.current().value)].parent = parent;
			tokenizer.next();
		}
	}

	tokenizer.next();
}

void readKeyword(anim::Tokenizer& tokenizer, std::vector<Joint>& bones) {
	// get next token
	auto token = tokenizer.current();

	// check the version
	if(token.value == ":version") {
		if(tokenizer.next().value != "1.10")
			throw std::runtime_error("only version 1.10 is supported (" + tokenizer.next().value + " found)");
		tokenizer.next();
	}

	// ignore the name
	else if(token.value == ":name") {
		tokenizer.next();
		tokenizer.next();
	}

	// reading the units
	else if(token.value == ":units")
		readUnits(tokenizer);

	// reading the documentation
	else if(token.value == ":documentation")
		readDocumentation(tokenizer);

	else if(token.value == ":root")
		readRoot(tokenizer, bones);

	else if(token.value == ":bonedata")
		readBoneData(tokenizer, bones);

	else if(token.value == ":hierarchy")
		readHierarchy(tokenizer, bones);

	else
		throw std::runtime_error("unknown token '" + token.value + "' on line " +
		                         boost::lexical_cast<std::string>(token.line));
}
}  // namespace

std::unique_ptr<anim::Skeleton> doLoad(const boost::filesystem::path& filename) {
	// open the file
	std::ifstream in(filename.string());
	AsfTokenizer tokenizer(in);

	// read the joints from the input file
	std::vector<Joint> joints;
	while(!tokenizer.eof())
		readKeyword(tokenizer, joints);

	// and, finally, transfer the information from joints to a Skeleton instance
	std::unique_ptr<anim::Skeleton> result(new anim::Skeleton());
	result->attributes()["type"] = "asf";

	for(const auto& j : joints) {
		std::size_t index;

		// root is simple
		if(j.parent < 0) {
			result->addRoot(j.name, j.position);
			index = 0;
		}

		// non-roots require a search
		else {
			int parentJoint = -1;
			for(unsigned ji = 0; ji < result->size(); ++ji)
				if((*result)[ji].name() == joints[j.parent].name)
					parentJoint = ji;
			assert(parentJoint >= 0);

			// both the position and rotation are at this stage in *world space*
			index = result->addChild((*result)[parentJoint],
			                         anim::Transform(j.axis.rotation, joints[j.parent].position), j.name);
		}

		(*result)[index].attributes() = j.attrs;
	}

	// change the positions to be in world space
	for(auto& j : *result)
		if(j.hasParent())
			j.tr().translation += j.parent().tr().translation;

	// and convert to local space
	for(int a = result->size() - 1; a > 0; --a)
		(*result)[a].tr() = (*result)[a].parent().tr().inverse() * (*result)[a].tr();

	return result;
}

//////////

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<anim::Skeleton> a_skel;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename()))
		data.set(a_skel, *doLoad(filename.filename()));
	else {
		data.set(a_skel, anim::Skeleton());
		out.addError("Cannot load filename " + filename.filename().string());
	}

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename",
	                  possumwood::Filename({
	                      "ASF files (*.asf)",
	                  }));
	meta.addAttribute(a_skel, "skeleton", anim::Skeleton(), possumwood::AttrFlags::kVertical);

	meta.addInfluence(a_filename, a_skel);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/loaders/asf", init);
