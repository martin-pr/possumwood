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

bool Graph::Path::empty() const {
	return n_links.empty();
}

void Graph::Path::add(const V2i& link) {
	n_links.push_back(link);
}

void Graph::Path::clear() {
	n_links.clear();
}

Graph::Path::const_iterator Graph::Path::begin() const {
	return n_links.rbegin();
}

Graph::Path::const_iterator Graph::Path::end() const {
	return n_links.rend();
}

const V2i& Graph::Path::front() const {
	assert(!n_links.empty());
	return n_links.back();
}

const V2i& Graph::Path::back() const {
	assert(!n_links.empty());
	return n_links.front();
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

void Graph::step(BFSVisitors& visited, std::deque<V2i>& q, const V2i& current_v, const V2i& new_v) const {
	if(!visited.visited(Index{new_v, 0})) {
		if(m_nLinks.edge(current_v, new_v).residualCapacity() > 0) {
			visited.visit(Index{new_v, 0}, Index{current_v, 0});
			q.push_back(new_v);
		}
	}
}

int Graph::bfs_2(Path& path, std::size_t& offset) const {
	path.clear();

	BFSVisitors visited(m_size, 1);
	std::deque<V2i> q;

	const std::size_t end = m_size.x * m_size.y;
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t src_id = (i + offset) % end;
		const V2i src_v = i2v(src_id);

		if(m_sourceLinks.edge(src_v).residualCapacity() > 0) {
			visited.clear();
			visited.visit(Index{src_v, 0}, Index{V2i(-1, -1), 0});

			q.clear();
			q.push_back(src_v);

			// the core of the algorithm
			while(!q.empty()) {
				V2i current_v = q.front();
				q.pop_front();

				// check if there is an exit point here
				{
					int flow = m_sinkLinks.edge(current_v).residualCapacity();
					if(flow > 0) {
						path.clear();

						// this is SLOW, but for now will do
						while(current_v != V2i(-1, -1)) {
							path.add(current_v);

							const Index& parent_v = visited.parent(Index{current_v, 0});
							if(parent_v.index != V2i(-1, -1))
								flow = std::min(flow, m_nLinks.edge(parent_v.index, current_v).residualCapacity());

							current_v = parent_v.index;
						}

						flow = std::min(flow, m_sourceLinks.edge(path.front()).residualCapacity());

						assert(path.isValid());

						offset = src_id+1;

						return flow;
					}
				}

				// try to move horizontally left
				if(current_v.x > 0)
					step(visited, q, current_v, V2i(current_v.x-1, current_v.y));

				// try to move horizontally right
				if(current_v.x < m_size.x-1)
					step(visited, q, current_v, V2i(current_v.x+1, current_v.y));

				// try to move vertically up
				if(current_v.y > 0)
					step(visited, q, current_v, V2i(current_v.x, current_v.y-1));

				// try to move vertically down
				if(current_v.y < m_size.y-1)
					step(visited, q, current_v, V2i(current_v.x, current_v.y+1));
			}
		}
	}

	return 0;
}

int Graph::flow(const Path& path) const {
	assert(path.isValid());

	int result = m_sourceLinks.edge(path.front()).residualCapacity();

	if(!path.empty()) {
		auto it1 = path.begin();
		auto it2 = it1+1;

		while(it2 != path.end()) {
			result = std::min(result, m_nLinks.edge(*it1, *it2).residualCapacity());

			++it1;
			++it2;
		}
	}

	result = std::min(result, m_sinkLinks.edge(path.back()).residualCapacity());

	return result;
}

void Graph::solve() {
	Path path;

	std::size_t counter = 0;

	std::size_t offset = 0;

	int flow;
	while((flow = bfs_2(path, offset))) {
		assert(path.isValid());
		assert(this->flow(path) == flow);

		assert(flow > 0 && "Augmented path flow at the beginning of each iteration should be positive");

		// update the graph
		m_sourceLinks.edge(path.front()).addFlow(flow);

		if(!path.empty()) {
			auto it1 = path.begin();
			auto it2 = it1+1;
			while(it2 != path.end()) {
				m_nLinks.edge(*it1, *it2).addFlow(flow);

				assert(m_nLinks.edge(*it1, *it2).residualCapacity() >= 0);
				assert(m_nLinks.edge(*it2, *it1).residualCapacity() >= 0);

				++it1;
				++it2;
			}
		}

		m_sinkLinks.edge(path.back()).addFlow(flow);

		assert(this->flow(path) == 0 && "Augmented path flow at the end of each iteration should be zero");

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
