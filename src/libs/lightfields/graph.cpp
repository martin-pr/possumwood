#include "graph.h"

#include <cassert>

namespace lightfields {

Graph::Path::Path() : source(-1, -1), sink(-1, -1) {
}

namespace {

int sqdist(const Imath::V2i& v1, const Imath::V2i& v2) {
	return (v2[0]-v1[0]) * (v2[0]-v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]);
}

}

bool Graph::Path::isValid() const {
	bool result = source[0] >= 0 && source[1] >= 0 && sink[0] >= 0 && sink[1] >= 0;

	if(n_links.empty())
		result &= (sqdist(source, sink) == 0);

	else {
		result &= (sqdist(source, n_links.front()) == 0);

		if(!n_links.empty()) {
			auto it1 = n_links.begin();
			auto it2 = it1+1;
			while(it2 != n_links.end()) {
				result &= (sqdist(*it1, *it2) == 1);

				++it1;
				++it2;
			}
		}

		result &= (sqdist(sink, n_links.back()) == 0);
	}

	return result;
}


/////

Graph::Graph(const Imath::V2i& size, float n_link_value) : m_size(size), m_sourceLinks(size), m_sinkLinks(size), m_nLinks(size, n_link_value) {
}

void Graph::setValue(const Imath::V2i& pos, float source_weight, float sink_weight) {
	m_sourceLinks.edge(pos).forward = source_weight;
	m_sinkLinks.edge(pos).forward = sink_weight;
}

bool Graph::dfs(Path& path) const {
	path.n_links.clear();
	std::set<Imath::V2i, SetComparator> visited;

	for(int siy=0; siy<m_size[1]; ++siy) {
		for(int six=0; six<m_size[0]; ++six) {
			if(m_sourceLinks.edge(Imath::V2i(six, siy)).forward > 0.0f) {
				path.n_links.clear();

				path.source = Imath::V2i(six, siy);
				path.n_links.push_back(path.source);

				visited.clear();
				visited.insert(path.source);

				if(dfs_2(path, visited, path.source)) {
					assert(path.isValid());
					return true;
				}
			}
		}
	}

	return false;
}

bool Graph::dfs_2(Path& path, std::set<Imath::V2i, SetComparator>& visited, const Imath::V2i& index) const {
	// first, try to see if there is a way out at the current pixel
	if(m_sinkLinks.edge(index).forward > 0.0f) {
		path.sink = index;
		assert(path.isValid());
		return true;
	}

	// try to move horizontally left
	if(index[0] > 0) {
		const Imath::V2i new_index(index[0]-1, index[1]);
		if(visited.find(new_index) == visited.end()) {
			visited.insert(new_index);

			if(m_nLinks.edge(index, new_index) > 0.0f) {
				path.n_links.push_back(new_index);

				if(dfs_2(path, visited, new_index)) {
					assert(path.isValid());
					return true;
				}
				else
					path.n_links.pop_back();
			}
		}
	}

	// try to move horizontally right
	if(index[0] < m_size[0]-1) {
		const Imath::V2i new_index(index[0]+1, index[1]);
		if(visited.find(new_index) == visited.end()) {
			visited.insert(new_index);

			if(m_nLinks.edge(index, new_index) > 0.0f) {
				path.n_links.push_back(new_index);

				if(dfs_2(path, visited, new_index)) {
					assert(path.isValid());
					return true;
				}
				else
					path.n_links.pop_back();
			}
		}
	}

	// try to move vertically up
	if(index[1] > 0) {
		const Imath::V2i new_index(index[0], index[1]-1);
		if(visited.find(new_index) == visited.end()) {
			visited.insert(new_index);

			if(m_nLinks.edge(index, new_index) > 0.0f) {
				path.n_links.push_back(new_index);

				if(dfs_2(path, visited, new_index)) {
					assert(path.isValid());
					return true;
				}
				else
					path.n_links.pop_back();
			}
		}
	}

	// try to move vertically down
	if(index[1] < m_size[1]-1) {
		const Imath::V2i new_index(index[0], index[1]+1);
		if(visited.find(new_index) == visited.end()) {
			visited.insert(new_index);

			if(m_nLinks.edge(index, new_index) > 0.0f) {
				path.n_links.push_back(new_index);

				if(dfs_2(path, visited, new_index)) {
					assert(path.isValid());
					return true;
				}
				else
					path.n_links.pop_back();
			}
		}
	}

	return false;
}

float Graph::flow(const Path& path) const {
	assert(path.isValid());

	float result = m_sourceLinks.edge(path.source).forward;

	if(!path.n_links.empty()) {
		auto it1 = path.n_links.begin();
		auto it2 = it1+1;

		while(it2 != path.n_links.end()) {
			result = std::min(result, m_nLinks.edge(*it1, *it2));

			++it1;
			++it2;
		}
	}

	result = std::min(result, m_sinkLinks.edge(path.sink).forward);

	return result;
}

void Graph::collect(std::set<Imath::V2i, Graph::SetComparator>& subgraph, const Imath::V2i& i) const {
	if(subgraph.find(i) == subgraph.end()) {
		subgraph.insert(i);

		if(i[0] > 0 && m_nLinks.edge(i, Imath::V2i(i[0]-1, i[1])) > 0.0f)
			collect(subgraph, Imath::V2i(i[0]-1, i[1]));

		if(i[0] < m_size[0]-1 && m_nLinks.edge(i, Imath::V2i(i[0]+1, i[1])) > 0.0f)
			collect(subgraph, Imath::V2i(i[0]+1, i[1]));

		if(i[1] > 0 && m_nLinks.edge(i, Imath::V2i(i[0], i[1]-1)) > 0.0f)
			collect(subgraph, Imath::V2i(i[0], i[1]-1));

		if(i[1] < m_size[1]-1 && m_nLinks.edge(i, Imath::V2i(i[0], i[1]+1)) > 0.0f)
			collect(subgraph, Imath::V2i(i[0], i[1]+1));
	}
}

void Graph::solve() {
	//////////
	// recursive path search

	Path path;
	while(dfs(path)) {
		assert(path.isValid());

		// get the maximum flow through the path
		const float f = flow(path);
		assert(f > 0.0f);

		// update the graph
		{
			{
				auto& e = m_sourceLinks.edge(path.source);
				e.forward -= f;
				e.backward += f;
			}

			if(!path.n_links.empty()) {
				// m_nLinks.edge(path.source, path.n_links.front()) -= f;
				// m_nLinks.edge(path.n_links.front(), path.source) += f;

				auto it1 = path.n_links.begin();
				auto it2 = it1+1;
				while(it2 != path.n_links.end()) {
					m_nLinks.edge(*it1, *it2) -= f;
					m_nLinks.edge(*it2, *it1) += f;

					++it1;
					++it2;
				}

				// m_nLinks.edge(path.n_links.front(), path.sink) -= f;
				// m_nLinks.edge(path.sink, path.n_links.front()) += f;
			}

			{
				auto& e = m_sinkLinks.edge(path.sink);
				e.forward -= f;
				e.backward += f;
			}
		}

		assert(flow(path) == 0.0f);
	}
}

std::set<Imath::V2i, Graph::SetComparator> Graph::sourceGraph() const {
	std::set<Imath::V2i, Graph::SetComparator> result;
	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x)
			if(m_sourceLinks.edge(Imath::V2i(x, y)).forward > 0.0f)
				collect(result, Imath::V2i(x, y));

	return result;
}

std::set<Imath::V2i, Graph::SetComparator> Graph::sinkGraph() const {
	std::set<Imath::V2i, Graph::SetComparator> result;
	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x)
			if(m_sinkLinks.edge(Imath::V2i(x, y)).backward > 0.0f)
				collect(result, Imath::V2i(x, y));

	return result;
}

}
