#include "obj.h"

#include <array>
#include <cassert>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include "builder.h"
#include "obj.h"

namespace possumwood {

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

class FaceAdaptor {
  public:
	class Vertices {
	  private:
		static int convert(const FaceIndex& fi) {
			return fi.v - 1;
		}

	  public:
		Vertices(const Face* face) : m_face(face) {
			assert(face != nullptr);
		}

		typedef boost::
		    transform_iterator<std::function<int(const FaceIndex&)>, std::vector<FaceIndex>::const_iterator, int, int>
		        const_iterator;

		const_iterator begin() const {
			return const_iterator(m_face->indices.begin(), convert);
		}

		const_iterator end() const {
			return const_iterator(m_face->indices.end(), convert);
		}

		const Face* m_face;
	};

  private:
	static const Vertices convert(const Face& face) {
		return Vertices(&face);
	}

  public:
	FaceAdaptor(const std::vector<Face>* faces) : m_faces(faces) {
	}

	typedef boost::
	    transform_iterator<std::function<Vertices(const Face&)>, std::vector<Face>::const_iterator, Vertices, Vertices>
	        const_iterator;

	const_iterator begin() const {
		return const_iterator(m_faces->begin(), convert);
	}

	const_iterator end() const {
		return const_iterator(m_faces->end(), convert);
	}

	std::size_t size() const {
		return m_faces->size();
	}

  private:
	const std::vector<Face>* m_faces;
};

std::istream& operator>>(std::istream& in, FaceIndex& f) {
	f.v = -1;
	f.vn = -1;
	f.vt = -1;

	in >> f.v;
	if(in.peek() == '/') {
		in.get();

		if(in.peek() != '/')
			in >> f.vt;

		if(in.peek() == '/') {
			in.get();

			if(in.peek() >= '0' && in.peek() <= '9')
				in >> f.vn;
		}

		assert(in.peek() != '/');
	}

	return in;
}

Mesh makeMesh(const std::string& name, const std::vector<std::array<float, 3>>& v, const std::vector<Face>& f) {
	Mesh result(name);
	Mesh::MeshData& mesh = result.edit();

	// build the base polymesh
	possumwood::CGALBuilder<possumwood::CGALPolyhedron::HalfedgeDS, typeof(v), FaceAdaptor> builder(v, FaceAdaptor(&f));
	mesh.polyhedron().delegate(builder);

	return result;
}

void addNormals(Mesh& mesh,
                const std::string& attrName,
                const std::vector<std::array<float, 3>>& n,
                const std::vector<Face>& f) {
	auto& editableMesh = mesh.edit();

	auto& normals = editableMesh.halfedgeProperties().addProperty(attrName, std::array<float, 3>{{0, 0, 0}});

	auto face = f.begin();
	for(auto fit = editableMesh.polyhedron().facets_begin(); fit != editableMesh.polyhedron().facets_end(); ++fit) {
		assert(fit->facet_degree() == face->indices.size());

		auto hit = fit->facet_begin();
		for(std::size_t hi = 0; hi < fit->facet_degree(); ++hi) {
			normals.set(hit->property_key(), n[face->indices[hi].vn - 1]);

			++hit;
		}

		++face;
	}
}

void addUVs(Mesh& mesh,
            const std::string& attrName,
            const std::vector<std::array<float, 2>>& uv,
            const std::vector<Face>& f) {
	auto& editableMesh = mesh.edit();

	auto& uvs = editableMesh.halfedgeProperties().addProperty(attrName, std::array<float, 2>{{0, 0}});

	auto face = f.begin();
	for(auto fit = editableMesh.polyhedron().facets_begin(); fit != editableMesh.polyhedron().facets_end(); ++fit) {
		assert(fit->facet_degree() == face->indices.size());

		auto hit = fit->facet_begin();
		for(std::size_t hi = 0; hi < fit->facet_degree(); ++hi) {
			uvs.set(hit->property_key(), uv[face->indices[hi].vt - 1]);

			++hit;
		}

		++face;
	}
}

}  // namespace

Mesh loadObj(boost::filesystem::path path,
             const std::string& name,
             const std::string& normalsAttr,
             const std::string& uvsAttr) {
	if(!boost::filesystem::exists(path))
		throw std::runtime_error("File " + path.string() + " doesn't exist");

	std::vector<std::array<float, 3>> vertices, normals;
	std::vector<std::array<float, 2>> uvs;
	std::vector<Face> faces;

	std::size_t vertexOrigin = 0;
	std::size_t normalOrigin = 0;
	std::size_t uvOrigin = 0;
	std::size_t objectId = 0;

	std::ifstream file(path.string());

	while(!file.eof()) {
		std::string line;
		std::getline(file, line);

		if(!line.empty() && line[0] != '#') {
			std::stringstream linestr(line);

			std::string id;
			linestr >> id;

			if(id == "o") {
				if(vertices.size() != vertexOrigin || normalOrigin != normals.size()) {
					vertexOrigin = vertices.size();
					normalOrigin = normals.size();
					uvOrigin = uvs.size();

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
			else if(id == "vt") {
				std::array<float, 2> t;
				linestr >> t[0] >> t[1];

				uvs.push_back(t);
			}
			else if(id == "f") {
				Face f;
				f.objectId = objectId;

				while(!linestr.eof()) {
					FaceIndex fi;
					linestr >> fi;

					if(fi.vn >= 0)
						fi.vn += normalOrigin;

					if(fi.vt > 0)
						fi.vt += uvOrigin;

					if(fi.v > 0) {
						fi.v += vertexOrigin;

						// support for per-vertex normals, without explicit index in "f" record
						if(fi.vn < 0 && fi.v + normalOrigin <= normals.size())
							fi.vn = fi.v + normalOrigin;

						f.indices.push_back(fi);
					}
				}

				assert(f.indices.size() >= 3);

				faces.push_back(f);
			}
		}
	}

	auto mesh = makeMesh(name, vertices, faces);

	if(!normals.empty())
		addNormals(mesh, normalsAttr, normals, faces);

	if(!uvs.empty())
		addUVs(mesh, uvsAttr, uvs, faces);

	return mesh;
}

}  // namespace possumwood
