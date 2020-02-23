#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

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

bool Graph::bfs(Path& path) const {
	path.n_links.clear();

	std::map<Imath::V2i, Imath::V2i, SetComparator> visited;
	std::queue<Imath::V2i> q;

	// set the "origin" visited nodes
	for(int siy=0; siy<m_size[1]; ++siy) {
		for(int six=0; six<m_size[0]; ++six) {
			if(m_sourceLinks.edge(Imath::V2i(six, siy)).forward > 0.0f) {
				visited[Imath::V2i(six, siy)] = Imath::V2i(-1, -1);
				q.push(Imath::V2i(six, siy));
			}
		}
	}

	// the core of the algorithm
	while(!q.empty()) {
		Imath::V2i current = q.front();
		q.pop();

		// check if there is an exit point here
		if(m_sinkLinks.edge(current).forward > 0.0f) {
			path.n_links.clear();

			path.sink = current;

			// this is SLOW, but for now will do
			while(current[0] >= 0) {
				path.n_links.insert(path.n_links.begin(), current);
				current = visited[current];
			}

			path.source = path.n_links.front();

			return true;
		}

		// try to move horizontally left
		if(current[0] > 0) {
			const Imath::V2i new_index(current[0]-1, current[1]);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index) > 0.0f)
					q.push(new_index);
			}
		}

		// try to move horizontally left
		if(current[0] < m_size[0]-1) {
			const Imath::V2i new_index(current[0]+1, current[1]);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index) > 0.0f)
					q.push(new_index);
			}
		}

		// try to move vertically up
		if(current[1] > 0) {
			const Imath::V2i new_index(current[0], current[1]-1);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index) > 0.0f)
					q.push(new_index);
			}
		}

		// try to move vertically down
		if(current[1] < m_size[1]-1) {
			const Imath::V2i new_index(current[0], current[1]+1);
			if(visited.find(new_index) == visited.end()) {
				visited[new_index] = current;

				if(m_nLinks.edge(current, new_index) > 0.0f)
					q.push(new_index);
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
	while(bfs(path)) {
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
				auto it1 = path.n_links.begin();
				auto it2 = it1+1;
				while(it2 != path.n_links.end()) {
					m_nLinks.edge(*it1, *it2) -= f;
					m_nLinks.edge(*it2, *it1) += f;

					++it1;
					++it2;
				}
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
