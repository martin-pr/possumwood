#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

#include "bfs_visitors.h"

namespace lightfields {

Graph::Path::Path() {
}

namespace {

int sqdist(const V2i& v1, const V2i& v2) {
	return (v2.x-v1.x) * (v2.x-v1.x) + (v2.y-v1.y) * (v2.y-v1.y);
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

Graph::Graph(const V2i& size, int n_link_value) : m_size(size), m_sourceLinks(size), m_sinkLinks(size), m_nLinks(size, n_link_value) {
}

void Graph::setValue(const V2i& pos, int source_weight, int sink_weight) {
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

std::size_t Graph::v2i(const V2i& v) const {
	return v.x + v.y*m_size.x;
}

V2i Graph::i2v(std::size_t v) const {
	v = v % (m_size.x*m_size.y);
	return V2i(v % m_size.x, v / m_size.x);
}

bool Graph::bfs_2(Path& path, std::size_t& offset) const {
	path.n_links.clear();

	BFSVisitors visited(m_size.x * m_size.y);
	std::deque<std::size_t> q;

	const std::size_t end = m_size.x * m_size.y;
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t src_id = (i + offset) % end;
		const V2i src_v = i2v(src_id);

		if(m_sourceLinks.edge(src_v).residualCapacity() > 0) {
			visited.clear();
			visited.visit(src_id, std::numeric_limits<std::size_t>::max());

			q.clear();
			q.push_back(src_id);

			// the core of the algorithm
			while(!q.empty()) {
				std::size_t current_id = q.front();
				V2i current_v = i2v(current_id);
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
				if(current_v.x > 0) {
					const V2i new_v(current_v.x-1, current_v.y);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move horizontally right
				if(current_v.x < m_size.x-1) {
					const V2i new_v(current_v.x+1, current_v.y);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move vertically up
				if(current_v.y > 0) {
					const V2i new_v(current_v.x, current_v.y-1);
					const std::size_t new_id = v2i(new_v);
					if(!visited.visited(new_id)) {
						if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
							visited.visit(new_id, current_id);
							q.push_back(new_id);
						}
					}
				}

				// try to move vertically down
				if(current_v.y < m_size.y-1) {
					const V2i new_v(current_v.x, current_v.y+1);
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
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);
	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x)
			result.at<unsigned char>(y, x) = 127;

	// reachable from source as a trivial DFS search
	{
		std::vector<V2i> stack;
		for(int y=0; y<m_size.y; ++y)
			for(int x=0; x<m_size.x; ++x)
				if(m_sourceLinks.edge(V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(V2i(x, y));

					while(!stack.empty()) {
						const V2i current = stack.back();
						stack.pop_back();

						assert(m_sinkLinks.edge(current).residualCapacity() == 0 && "TODO: FIX THIS TRIGGER!!!");

						unsigned char& value = result.at<unsigned char>(current.y, current.x);
						if(value < 255) {
							result.at<unsigned char>(current.y, current.x) = 255;

							if(current.x > 0 && m_nLinks.edge(current, V2i(current.x-1, current.y)).residualCapacity() > 0)
								stack.push_back(V2i(current.x-1, current.y));
							if(current.x < m_size.x-1 && m_nLinks.edge(current, V2i(current.x+1, current.y)).residualCapacity() > 0)
								stack.push_back(V2i(current.x+1, current.y));

							if(current.y > 0 && m_nLinks.edge(current, V2i(current.x, current.y-1)).residualCapacity() > 0)
								stack.push_back(V2i(current.x, current.y-1));
							if(current.y < m_size.y-1 && m_nLinks.edge(current, V2i(current.x, current.y+1)).residualCapacity() > 0)
								stack.push_back(V2i(current.x, current.y+1));
						}
					}
				}
	}

	// reachable from sink as a trivial DFS search
	// (this is unnecessary for the actual solution and should eventually be removed)
	{
		std::vector<V2i> stack;
		for(int y=0; y<m_size.y; ++y)
			for(int x=0; x<m_size.x; ++x)
				if(m_sinkLinks.edge(V2i(x, y)).residualCapacity() > 0) {
					stack.push_back(V2i(x, y));

					while(!stack.empty()) {
						const V2i current = stack.back();
						stack.pop_back();

						assert(m_sourceLinks.edge(V2i(x, y)).residualCapacity() == 0);

						unsigned char& value = result.at<unsigned char>(current.y, current.x);
						if(value > 0) {
							result.at<unsigned char>(current.y, current.x) = 0;

							if(current.x > 0 && m_nLinks.edge(V2i(current.x-1, current.y), current).residualCapacity() > 0)
								stack.push_back(V2i(current.x-1, current.y));
							if(current.x < m_size.x-1 && m_nLinks.edge(V2i(current.x+1, current.y), current).residualCapacity() > 0)
								stack.push_back(V2i(current.x+1, current.y));

							if(current.y > 0 && m_nLinks.edge(V2i(current.x, current.y-1), current).residualCapacity() > 0)
								stack.push_back(V2i(current.x, current.y-1));
							if(current.y < m_size.y-1 && m_nLinks.edge(V2i(current.x, current.y+1), current).residualCapacity() > 0)
								stack.push_back(V2i(current.x, current.y+1));
						}
					}
				}
	}

	return result;
}

cv::Mat Graph::sourceFlow() const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = m_sourceLinks.edge(V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::sinkFlow() const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = m_sinkLinks.edge(V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::horizontalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x-1; ++x) {
			auto& e = m_nLinks.edge(V2i(x, y), V2i(x+1, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::verticalFlow() const {
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y-1; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = m_nLinks.edge(V2i(x, y), V2i(x, y+1));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

}
