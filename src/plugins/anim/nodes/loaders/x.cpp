#include <fstream>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#include <possumwood_sdk/node_implementation.h>
#include <possumwood_sdk/datatypes/filename.h>
#include <actions/io.h>

#include "datatypes/skinned_mesh.h"
#include "datatypes/transform.h"
#include "datatypes/skeleton.h"
#include "tokenizer.h"

#include <ImathMatrixAlgo.h>

namespace openanim {

namespace {

class XTokenizer : public anim::Tokenizer {
  private:
	State start, commentStart, comment;

  public:
	XTokenizer(std::istream& in) : anim::Tokenizer(in), start(this), commentStart(this), comment(this) {
		start = [this](char c) {
			// don't read any space-like characters, but each is considered an end of a token
			if(std::isspace(c)) {
				emit();
				reject();
			}

			// { and } are tokens on their own
			else if((c == '{') || (c == '}') || (c == ',') || (c == ';')) {
				emit();
				accept(c);
				emit();
			}

			// / switches to commentStart
			else if(c == '/') {
				reject();
				commentStart.setActive();
			}

			// anything else is part of a token
			else
				accept(c);
		};

		commentStart = [this](char c) {
			// two / characters start a comment
			if(c == '/') {
				reject();
				comment.setActive();
			}
			// anything else after a / means accepting / and c and not starting a comment
			else {
				accept('/');
				accept(c);
				start.setActive();
			}
		};

		comment = [this](char c) {
			if(c == '\n')
				start.setActive();
			else
				reject();
		};

		// set start as the active state
		start.setActive();

		// read the first keyword into current
		next();
	}

  protected:
  private:
};

void readHeader(anim::Tokenizer& tokenizer) {
	assert(tokenizer.current() == "xof");

	tokenizer >> "0303txt";
	tokenizer >> "0032";

	tokenizer.next();
}

void skipBracketedSection(anim::Tokenizer& tokenizer) {
	// assert(tokenizer.current() == "{");
	while(!tokenizer.eof() && (tokenizer.current() != "}"))
		if(tokenizer.next() == "{")
			skipBracketedSection(tokenizer);
		else
			;

	tokenizer.next();
}

void readTemplate(anim::Tokenizer& tokenizer) {
	assert(tokenizer.current() == "template");

	// skip the name
	tokenizer.next();

	// and skip the content
	tokenizer >> "{";
	skipBracketedSection(tokenizer);
}

void readNormals(anim::Tokenizer& tokenizer, anim::SkinnedMesh& model, const Imath::M44f& transform) {
	assert(tokenizer.current() == "MeshNormals");

	tokenizer >> "{";

	std::size_t normalCount;
	tokenizer >> normalCount >> ";";

	std::vector<Imath::V3f> normals;

	for(std::size_t v = 0; v < normalCount; ++v) {
		Imath::V3f norm;
		tokenizer >> norm.x >> ";" >> norm.y >> ";" >> norm.z >> ";";

		if(v < normalCount - 1)
			tokenizer >> ",";
		else
			tokenizer >> ";";

		normals.push_back((norm * transform).normalized());
	}

	// then, read the number of polygons
	std::size_t polyCount;
	tokenizer >> polyCount >> ";";

	// and read the polygons
	std::size_t polyIndex = 0;
	for(std::size_t p = 0; p < polyCount; ++p) {
		unsigned vertexCount;
		tokenizer >> vertexCount >> ";";

		if(vertexCount < 3)
			throw("found a polygon with less-than 3 vertices.");

		// and the polygon vertices
		std::array<std::size_t, 3> poly;

		tokenizer >> poly[0] >> ",";
		tokenizer >> poly[2] >> ",";

		// perform trivial triangulation
		for(unsigned v = 0; v < vertexCount - 2; ++v) {
			// shift the first index
			poly[1] = poly[2];
			// and read the second
			tokenizer >> poly[2];

			if(v < vertexCount - 3)
				tokenizer >> ",";
			else
				tokenizer >> ";";

			model.normals().push_back(normals[poly[0]]);
			model.normals().push_back(normals[poly[1]]);
			model.normals().push_back(normals[poly[2]]);

			++polyIndex;
		}

		if(p < polyCount - 1)
			tokenizer >> ",";
		else
			tokenizer >> ";";
	}

	assert(polyIndex == model.polygons().size());

	// skip the rest
	skipBracketedSection(tokenizer);
}

void readSkinWeights(anim::Tokenizer& tokenizer, anim::SkinnedMesh& result, std::map<std::string, unsigned>& boneIds) {
	assert(tokenizer.current() == "SkinWeights");

	tokenizer >> "{";

	while(tokenizer.current() != "}") {
		std::string boneName;
		tokenizer >> boneName >> ";";

		// strip it of " symbols
		assert(boneName.length() > 2);
		assert(boneName[0] == '"' && boneName[boneName.length() - 1] == '"');
		boneName = boneName.substr(1, boneName.length() - 2);

		auto boneId = boneIds.find(boneName);
		if(boneId == boneIds.end())
			boneId = boneIds.insert(std::make_pair(boneName, boneIds.size())).first;
		assert(boneId != boneIds.end());

		std::size_t weightCount;
		tokenizer >> weightCount >> ";";

		std::vector<std::size_t> vertices;
		for(std::size_t v = 0; v < weightCount; ++v) {
			vertices.push_back(boost::lexical_cast<std::size_t>(tokenizer.next().value));
			if(v < weightCount - 1)
				tokenizer >> ",";
			else
				tokenizer >> ";";
		}

		float weight;
		for(std::size_t v = 0; v < weightCount; ++v) {
			tokenizer >> weight;

			if(v < weightCount - 1)
				tokenizer >> ",";
			else
				tokenizer >> ";";

			anim::Skinning skin(result.vertices()[vertices[v]].skinning());
			skin.addWeight(boneId->second, weight);
			result.vertices()[vertices[v]].setSkinning(skin);
		}

		// skip a matrix at the end
		float tmp;
		for(unsigned a = 0; a < 15; ++a)
			tokenizer >> tmp >> ",";
		tokenizer >> tmp >> ";" >> ";";

		tokenizer.next();
	}

	tokenizer.next();
}

std::unique_ptr<anim::SkinnedMesh> readMesh(anim::Tokenizer& tokenizer,
                           const Imath::M44f& transform,
                           std::map<std::string, unsigned>& boneIds) {
	assert(tokenizer.current() == "Mesh");
	tokenizer >> "{";

	std::unique_ptr<anim::SkinnedMesh> result(new anim::SkinnedMesh());

	// first, read the number of vertices
	std::size_t vertexCount;
	tokenizer >> vertexCount >> ";";

	// then read a vertex position for each vertex
	for(std::size_t v = 0; v < vertexCount; ++v) {
		Imath::V3f vert;
		tokenizer >> vert.x >> ";" >> vert.y >> ";" >> vert.z >> ";";

		if(v < vertexCount - 1)
			tokenizer >> ",";
		else
			tokenizer >> ";";

		result->vertices().add(vert * transform);
	}

	// then, read the number of polygons
	std::size_t polyCount;
	tokenizer >> polyCount >> ";";

	// and read the polygons
	for(std::size_t p = 0; p < polyCount; ++p) {
		unsigned vertexCount;
		tokenizer >> vertexCount >> ";";

		if(vertexCount < 3)
			throw("found a polygon with less-than 3 vertices.");

		// and the polygon vertices
		std::array<std::size_t, 3> poly;

		tokenizer >> poly[0] >> ",";
		tokenizer >> poly[2] >> ",";

		// perform trivial triangulation
		for(unsigned v = 0; v < vertexCount - 2; ++v) {
			// shift the first index
			poly[1] = poly[2];
			// and read the second
			tokenizer >> poly[2];

			if(v < vertexCount - 3)
				tokenizer >> ",";
			else
				tokenizer >> ";";

			result->polygons().add(poly[0], poly[1], poly[2]);
		}

		if(p < polyCount - 1)
			tokenizer >> ",";
		else
			tokenizer >> ";";
	}

	tokenizer.next();

	// other bits of the mesh definition
	while(tokenizer.current() != "}") {
		// cout << tokenizer.current().value << endl;
		if(tokenizer.current() == "MeshNormals") {
			auto normTr = transform.inverse().transposed();
			normTr[0][3] = 0.0f;
			normTr[1][3] = 0.0f;
			normTr[2][3] = 0.0f;
			normTr[3][3] = 1.0f;
			readNormals(tokenizer, *result, normTr);
		} else if(tokenizer.current() == "SkinWeights")
			readSkinWeights(tokenizer, *result, boneIds);
		else {
			tokenizer >> "{";
			skipBracketedSection(tokenizer);
		}
	}

	tokenizer.next();

	return result;
}

struct Frame {
	std::string name;
	Imath::M44f tr;
	std::vector<std::unique_ptr<Frame>> children;
	unsigned skinningUseCount = 0;
};

std::unique_ptr<Frame> readFrame(anim::Tokenizer& tokenizer,
                                 std::vector<anim::SkinnedMesh>& models,
                                 const Imath::M44f& transform,
                                 std::map<std::string, unsigned>& boneIds);

Imath::M44f readTransformation(anim::Tokenizer& tokenizer) {
	assert(tokenizer.current() == "FrameTransformMatrix");
	tokenizer >> "{";

	Imath::M44f result;
	for(unsigned a = 0; a < 16; ++a) {
		tokenizer >> result[a / 4][a % 4];
		if(a < 15)
			tokenizer >> ",";
		else
			tokenizer >> ";" >> ";";
	}

	tokenizer >> "}";

	tokenizer.next();

	return result;
}

std::unique_ptr<Frame> readFrameInternal(anim::Tokenizer& tokenizer,
                                         std::vector<anim::SkinnedMesh>& models,
                                         const Imath::M44f& transform,
                                         const std::string& name,
                                         std::map<std::string, unsigned>& boneIds) {
	assert(tokenizer.current() == "{");

	std::unique_ptr<Frame> result(new Frame());
	result->name = name;

	tokenizer.next();
	while(tokenizer.current() != "}") {
		if(tokenizer.current() == "FrameTransformMatrix")
			result->tr = readTransformation(tokenizer);

		else if(tokenizer.current() == "Mesh") {
			models.push_back(*readMesh(tokenizer, result->tr * transform, boneIds));
			models.back().setName(name);
		}

		else if(tokenizer.current() == "Frame")
			result->children.push_back(readFrame(tokenizer, models, result->tr * transform, boneIds));

		else
			throw std::runtime_error("unknown token " + tokenizer.current().value + " inside a Frame section");
	}

	assert(tokenizer.current() == "}");
	tokenizer.next();

	result->tr = result->tr * transform;

	return result;
}

std::unique_ptr<Frame> readFrame(anim::Tokenizer& tokenizer,
                                 std::vector<anim::SkinnedMesh>& models,
                                 const Imath::M44f& transform,
                                 std::map<std::string, unsigned>& boneIds) {
	assert(tokenizer.current() == "Frame");

	// read the name
	std::string name;
	tokenizer >> name;

	// and read the content
	tokenizer >> "{";
	std::unique_ptr<Frame> frame = readFrameInternal(tokenizer, models, transform, name, boneIds);

	return frame;
}

anim::Transform convertMatrix(const Imath::M44f& m) {
	// just discard the rotation and scale, and return only the translation (rotation and scale is irrelevant for
	// the animation rig anyways)
	return Imath::M44f(1, 0, 0, m[3][0], 0, 1, 0, m[3][1], 0, 0, 1, m[3][2], 0, 0, 0, 1);
}
}

struct RigData {
	std::vector<anim::SkinnedMesh> models;
	anim::Skeleton skeleton;
};

std::unique_ptr<RigData>
doLoad(const boost::filesystem::path& filename) {
	// instantiate the tokenizer
	std::ifstream in(filename.string());
	XTokenizer tokenizer(in);

	std::unique_ptr<RigData> result(new RigData());
	std::unique_ptr<Frame> skeleton;
	std::map<std::string, unsigned> skinningBoneIds;

	while(!tokenizer.eof()) {
		if(tokenizer.current() == "xof")
			readHeader(tokenizer);

		else if(tokenizer.current() == "template")
			readTemplate(tokenizer);

		else if(tokenizer.current() == "Frame")
			skeleton = readFrame(tokenizer, result->models, Imath::M44f(), skinningBoneIds);

		else
			throw std::runtime_error("unknown keyword " + tokenizer.current().value);
	}

	// convert the skeleton instance to result's Skeleton
	if(skeleton != NULL) {
		//////////
		// convert the skeleton data to a Skeleton instance

		// make the root
		result->skeleton.addRoot(skeleton->name, convertMatrix(skeleton->tr));

		// the BFS building functor
		std::function<void(std::size_t, const Frame&)> fn = [&fn, &result](std::size_t jointIndex, const Frame& frame) {
			// transfer all children
			for(auto& c : frame.children)
				result->skeleton.addChild(result->skeleton[jointIndex], convertMatrix(c->tr), c->name);
			assert(result->skeleton[jointIndex].children().size() == frame.children.size());

			// recursive call
			unsigned ctr = 0;
			for(auto& c : frame.children)
				fn(result->skeleton[jointIndex].children()[ctr++].index(), *c);
		};
		fn(0, *skeleton);

		// and convert to local space
		for(int b = result->skeleton.size() - 1; b > 0; --b)
			result->skeleton[b].tr() = result->skeleton[b].parent().tr().inverse() * result->skeleton[b].tr();

		///////
		// skinning data indices conversion

		// make the vector translating original skinning indices to new ones
		std::vector<unsigned> boneIndices;
		for(auto& j : result->skeleton) {
			auto it = skinningBoneIds.find(j.name());
			if(it != skinningBoneIds.end()) {
				while(boneIndices.size() <= it->second)
					boneIndices.push_back(0);

				boneIndices[it->second] = j.index();
			}
		}

		// and translate all skinning indices in all meshes to correspond to the Skeleton instance in result
		for(auto& m : result->models)
			for(auto& v : m.vertices()) {
				anim::Skinning skin = v.skinning();
				for(auto& s : skin)
					s = anim::Skinning::Weight(boneIndices[s.bone], s.weight);
				v.setSkinning(skin);
			}
	}

	return result;
}

/////

dependency_graph::InAttr<possumwood::Filename> a_filename;
dependency_graph::OutAttr<anim::Skeleton> a_skel;
dependency_graph::OutAttr<std::shared_ptr<const std::vector<anim::SkinnedMesh>>> a_meshes;

dependency_graph::State compute(dependency_graph::Values& data) {
	dependency_graph::State out;

	const possumwood::Filename filename = data.get(a_filename);

	if(!filename.filename().empty() && boost::filesystem::exists(filename.filename())) {
		std::unique_ptr<RigData> x = doLoad(filename.filename());

		data.set(a_skel, x->skeleton);
		data.set(a_meshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>(new std::vector<anim::SkinnedMesh>(x->models)));
	}
	else {
		data.set(a_skel, anim::Skeleton());
		data.set(a_meshes, std::shared_ptr<const std::vector<anim::SkinnedMesh>>());
		out.addError("Cannot load filename " + filename.filename().string());
	}

	return out;
}

void init(possumwood::Metadata& meta) {
	meta.addAttribute(a_filename, "filename", possumwood::Filename({
		"X files (*.x)",
	}));
	meta.addAttribute(a_skel, "skeleton");
	meta.addAttribute(a_meshes, "meshes");

	meta.addInfluence(a_filename, a_skel);
	meta.addInfluence(a_filename, a_meshes);

	meta.setCompute(compute);
}

possumwood::NodeImplementation s_impl("anim/loaders/x", init);

}
