#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

#include "bfs_visitors.h"

namespace lightfields {

Graph::Path::Path() {
}

namespace {

int sqdist(const Imath::V2i& v1, const Imath::V2i& v2) {
	return (v2[0]-v1[0]) * (v2[0]-v1[0]) + (v2[1]-v1[1]) * (v2[1]-v1[1]);
}

}

bool Graph::Path::isValid() const {
	if(n_links.empty())
		return false;

	else {
		bool result = true;

		auto it1 = n_links.begin();
		auto it2 = it1+1;
		while(it2 != n_links.end()) {
			result &= (sqdist(*it1, *it2) == 1);

			++it1;
			++it2;
		}
		return result;
	}
}


/////

Graph::Graph(const Imath::V2i& size, int n_link_value) : m_size(size), m_sourceLinks(size), m_sinkLinks(size), m_nLinks(size, n_link_value) {
}

void Graph::setValue(const Imath::V2i& pos, int source_weight, int sink_weight) {
	assert(source_weight >= 0);
	assert(sink_weight >= 0);

	// "first pass" - direct transfer between a sink an a source of the same pixel, which would otherwise have to be solved
	// in the BFS, but would always be the "first hit".
	const int flow = std::min(source_weight, sink_weight);

	{
		auto& e = m_sourceLinks.edge(pos);
		e.setCapacity(source_weight);
		if(flow > 0)
			e.addFlow(flow);
	}

	{
		auto& e = m_sinkLinks.edge(pos);
		e.setCapacity(sink_weight);
		if(flow > 0)
			e.addFlow(flow);
	}
}

std::size_t Graph::v2i(const Imath::V2i& v) const {
	return v[0] + v[1]*m_size[0];
}

Imath::V2i Graph::i2v(std::size_t v) const {
	v = v % (m_size[0]*m_size[1]);
	return Imath::V2i(v % m_size[0], v / m_size[0]);
}

bool Graph::bfs_2(Path& path, std::size_t& offset) const {
	path.n_links.clear();

	BFSVisitors visited(m_size[0] * m_size[1]);
	std::deque<std::size_t> q;

	const std::size_t end = m_size[0] * m_size[1];
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t src_id = (i + offset) % end;
		const Imath::V2i src_v = i2v(src_id);

		if(m_sourceLinks.edge(src_v).residualCapacity() > 0) {
			visited.clear();
			visited.visit(src_id, std::numeric_limits<std::size_t>::max());

			q.clear();
			q.push_back(src_id);

			// the core of the algorithm
			while(!q.empty()) {
				std::size_t current_id = q.front();
				Imath::V2i current_v = i2v(current_id);
				q.pop_front();

				// check if there is an exit point here
				if(m_sinkLinks.edge(current_v).residualCapacity() > 0) {
					path.n_links.clear();

					// this is SLOW, but for now will do
					while(current_id < std::numeric_limits<std::size_t>::max()) {
						path.n_links.push_back(current_v);

						current_id = visited.parent(current_id);
						current_v = i2v(current_id);
					}
					std::reverse(path.n_links.begin(), path.n_links.end());

					assert(path.isValid());

					offset = src_id+1;

					return true;
				}

				// try to move horizontally left
				if(current_v[0] > 0) {
					const Imath::V2i new_v(current_v[0]-1, current_v[1]);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move horizontally right
				if(current_v[0] < m_size[0]-1) {
					const Imath::V2i new_v(current_v[0]+1, current_v[1]);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move vertically up
				if(current_v[1] > 0) {
					const Imath::V2i new_v(current_v[0], current_v[1]-1);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move vertically down
				if(current_v[1] < m_size[1]-1) {
					const Imath::V2i new_v(current_v[0], current_v[1]+1);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}
			}
		}
	}

	return false;
}

int Graph::flow(const Path& path) const {
	assert(path.isValid());

	int result = m_sourceLinks.edge(path.n_links.front()).residualCapacity();

	if(!path.n_links.empty()) {
		auto it1 = path.n_links.begin();
		auto it2 = it1+1;

		while(it2 != path.n_links.end()) {
			result = std::min(result, m_nLinks.edge(*it1, *it2).residualCapacity());

			++it1;
			++it2;
		}
	}

	result = std::min(result, m_sinkLinks.edge(path.n_links.back()).residualCapacity());

	return result;
}

void Graph::solve() {
	Path path;

	std::size_t counter = 0;

	std::size_t offset = 0;
	while(bfs_2(path, offset)) {
		assert(path.isValid());


		// get the maximum flow through the path
		const int f = flow(path);
		assert(f > 0 && "Augmented path flow at the beginning of each iteration should be positive");

		// update the graph
		m_sourceLinks.edge(path.n_links.front()).addFlow(f);

		if(!path.n_links.empty()) {
			auto it1 = path.n_links.begin();
			auto it2 = it1+1;
			while(it2 != path.n_links.end()) {
				m_nLinks.edge(*it1, *it2).addFlow(f);

				assert(m_nLinks.edge(*it1, *it2).residualCapacity() >= 0);
				assert(m_nLinks.edge(*it2, *it1).residualCapacity() >= 0);

				++it1;
				++it2;
			}
		}

		m_sinkLinks.edge(path.n_links.back()).addFlow(f);

		assert(flow(path) == 0 && "Augmented path flow at the end of each iteration should be zero");

		++counter;
	}

	std::cout << "ST-cut solve with " << counter << " steps finished." << std::endl;
}

cv::Mat Graph::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);
	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x)
			result.at<unsigned char>(y, x) = 127;

	// reachable from source as a trivial DFS search
	{
		std::vector<Imath::V2i> stack;
		for(int y=0; y<m_size[1]; ++y)
			for(int x=0; x<m_size[0]; ++x)
				if(m_sourceLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(Imath::V2i(x, y));

					while(!stack.empty()) {
						const Imath::V2i current = stack.back();
						stack.pop_back();

						assert(m_sinkLinks.edge(current).residualCapacity() == 0 && "TODO: FIX THIS TRIGGER!!!");

						unsigned char& value = result.at<unsigned char>(current[1], current[0]);
						if(value < 255) {
							result.at<unsigned char>(current[1], current[0]) = 255;

							if(current[0] > 0 && m_nLinks.edge(current, Imath::V2i(current[0]-1, current[1])).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]-1, current[1]));
							if(current[0] < m_size[0]-1 && m_nLinks.edge(current, Imath::V2i(current[0]+1, current[1])).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]+1, current[1]));

							if(current[1] > 0 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]-1)).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]-1));
							if(current[1] < m_size[1]-1 && m_nLinks.edge(current, Imath::V2i(current[0], current[1]+1)).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]+1));
						}
					}
				}
	}

	// reachable from sink as a trivial DFS search
	// (this is unnecessary for the actual solution and should eventually be removed)
	{
		std::vector<Imath::V2i> stack;
		for(int y=0; y<m_size[1]; ++y)
			for(int x=0; x<m_size[0]; ++x)
				if(m_sinkLinks.edge(Imath::V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(Imath::V2i(x, y));

					while(!stack.empty()) {
						const Imath::V2i current = stack.back();
						stack.pop_back();

						assert(m_sourceLinks.edge(Imath::V2i(x, y)).residualCapacity() == 0);

						unsigned char& value = result.at<unsigned char>(current[1], current[0]);
						if(value > 0) {
							result.at<unsigned char>(current[1], current[0]) = 0;

							if(current[0] > 0 && m_nLinks.edge(Imath::V2i(current[0]-1, current[1]), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]-1, current[1]));
							if(current[0] < m_size[0]-1 && m_nLinks.edge(Imath::V2i(current[0]+1, current[1]), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0]+1, current[1]));

							if(current[1] > 0 && m_nLinks.edge(Imath::V2i(current[0], current[1]-1), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]-1));
							if(current[1] < m_size[1]-1 && m_nLinks.edge(Imath::V2i(current[0], current[1]+1), current).residualCapacity() > 0)
								stack.push_back(Imath::V2i(current[0], current[1]+1));
						}
					}
				}
	}

	return result;
}

cv::Mat Graph::sourceFlow() const {
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_sourceLinks.edge(Imath::V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::sinkFlow() const {
	cv::Mat result(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_sinkLinks.edge(Imath::V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::horizontalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]; ++y)
		for(int x=0; x<m_size[0]-1; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x+1, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::verticalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size[1], m_size[0], CV_8UC1);

	for(int y=0; y<m_size[1]-1; ++y)
		for(int x=0; x<m_size[0]; ++x) {
			auto& e = m_nLinks.edge(Imath::V2i(x, y), Imath::V2i(x, y+1));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

}
