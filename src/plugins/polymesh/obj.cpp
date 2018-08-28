#include "obj.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <array>

#include <boost/filesystem.hpp>

#include "generic_polymesh.inl"
#include "obj.h"

namespace possumwood {
namespace polymesh {

namespace {
	struct FaceIndex {
		int v = -1;
		int vn = -1;
		int vt = -1;
	};

	struct Face {
		std::vector<FaceIndex> indices;
		std::size_t objectId;
	};

	std::istream& operator >> (std::istream& in, FaceIndex& f) {
		f.v = -1;
		f.vn = -1;
		f.vt = -1;

		in >> f.v;
		if(in.peek() == '/') {
			in.get();

			if(in.peek() != '/')
				in >> f.vt;

			assert(in.peek() == '/');
			in.get();

			if(in.peek() >= '0' && in.peek() <= '9')
				in >> f.vn;

			assert(in.peek() != '/');
		}

		return in;
	}

	GenericPolymesh makeMesh(const std::vector<std::array<float, 3>>& v, const std::vector<Face>& f, std::size_t indexCount) {
		GenericPolymesh result;

		// copy the vertex data
		{
			// only one per-vertex attribute - position P
			GenericPolymesh::Vertices::Handle p = result.vertices().handles().
				add<std::array<float, 3>>("P", std::array<float, 3>{{0,0,0}});

			for(auto& vert : v) {
				auto vi = result.vertices().add();
				vi->set(p, vert);
			}
		}

		// copy the polygon data
		{
			std::vector<std::size_t> indices;

			// only one per-polygon attribute - object ID
			GenericPolymesh::Polygons::Handle objId = result.polygons().handles().
				add<std::size_t>("objectId", 0);

			for(auto& face : f) {
				indices.clear();
				for(auto& i : face.indices) {
					assert(i.v > 0);
					indices.push_back(i.v - 1);
				}

				auto fi = result.polygons().add(indices.begin(), indices.end());
				fi->set(objId, face.objectId);
			}
		}

		return result;
	}
}

GenericPolymesh loadObj(boost::filesystem::path path) {
	assert(boost::filesystem::exists(path));

	std::vector<std::array<float, 3>> vertices, normals;
	std::vector<Face> faces;

	std::size_t vertexOrigin = 0;
	std::size_t normalOrigin = 0;
	std::size_t objectId = 0;

	std::size_t indexCount = 0;

	std::ifstream file(path.string());

	while(!file.eof()) {
		std::string line;
		std::getline(file, line);

		if(!line.empty() && line[0] != '#') {
			std::stringstream linestr(line);

			std::string id;
			linestr >> id;

			if(id == "mtllib")
				;
			else if(id == "o") {
				if(vertices.size() != vertexOrigin || normalOrigin != normals.size()) {
					vertexOrigin = vertices.size();
					normalOrigin = normals.size();

					++objectId;
				}
			}
			else if(id == "v") {
				std::array<float, 3> v;
				linestr >> v[0] >> v[1] >> v[2];

				vertices.push_back(v);
			}
			else if(id == "vn") {
				std::array<float, 3> n;
				linestr >> n[0] >> n[1] >> n[2];

				normals.push_back(n);
			}
			else if(id == "f") {
				Face f;
				f.objectId = objectId;

				while(!linestr.eof()) {
					FaceIndex fi;
					linestr >> fi;

					if(fi.vn >= 0)
						fi.vn += normalOrigin;

					if(fi.v > 0) {
						fi.v += vertexOrigin;

						f.indices.push_back(fi);
					}
				}

				assert(f.indices.size() >= 3);
				indexCount += f.indices.size();

				faces.push_back(f);
			}
		}
	}

	return makeMesh(vertices, faces, indexCount);
}

}
}
