#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

#include "bfs_visitors.h"

namespace lightfields {

Graph::Path::Path() {
}

bool Graph::Path::isValid() const {
	if(n_links.empty())
		return false;

	else {
		bool result = true;

		auto it1 = n_links.begin();
		auto it2 = it1+1;
		while(it2 != n_links.end()) {
			result &= (Index::sqdist(*it1, *it2) == 1);

			++it1;
			++it2;
		}
		return result;
	}
}

bool Graph::Path::empty() const {
	return n_links.empty();
}

void Graph::Path::add(const Index& link) {
	assert(n_links.empty() || (Index::sqdist(link, n_links.back()) == 1));
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

const Index& Graph::Path::front() const {
	assert(!n_links.empty());
	return n_links.back();
}

const Index& Graph::Path::back() const {
	assert(!n_links.empty());
	return n_links.front();
}

/////

Graph::Graph(const V2i& size, int n_link_value, std::size_t layer_count) : m_size(size) {
	m_tLinks = std::vector<TLinks>(layer_count+1, TLinks(size));
	m_nLinks = std::vector<NLinks>(layer_count, NLinks(size, n_link_value));
}

void Graph::setValue(const V2i& pos, const std::vector<int>& values) {
	assert(values.size() == m_tLinks.size());

	auto tit = m_tLinks.begin();
	auto vit = values.begin();

	while(tit != m_tLinks.end()) {
		tit->edge(pos).setCapacity(*vit);

		assert(*vit >= 0);
		assert(tit->edge(pos).residualCapacity() == *vit);

		++tit;
		++vit;
	}
}

std::size_t Graph::v2i(const V2i& v) const {
	return v.x + v.y*m_size.x;
}

V2i Graph::i2v(std::size_t v) const {
	v = v % (m_size.x*m_size.y);
	return V2i(v % m_size.x, v / m_size.x);
}

void Graph::step(BFSVisitors& visited, std::deque<Index>& q, const Index& current_v, const Index& new_v) const {
	assert(current_v.n_layer == new_v.n_layer && "step() in the same layer only");
	assert(current_v.n_layer < m_nLinks.size());

	if(!visited.visited(new_v)) {
		if(m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0) {
			visited.visit(new_v, current_v);
			q.push_back(new_v);
		}
	}
}

int Graph::bfs_2(Path& path, std::size_t& offset) const {
	path.clear();

	BFSVisitors visited(m_size, m_nLinks.size());
	std::deque<Index> q;

	const std::size_t end = m_size.x * m_size.y;
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t src_id = (i + offset) % end;
		const Index src_v = Index{i2v(src_id), 0};

		assert(m_tLinks.front().edge(src_v.pos).capacity() >= 0);
		assert(m_tLinks.front().edge(src_v.pos).flow() >= 0);
		assert(m_tLinks.front().edge(src_v.pos).residualCapacity() >= 0);

		if(m_tLinks.front().edge(src_v.pos).residualCapacity() > 0) {
			visited.clear();
			visited.visit(src_v, Index{V2i(-1, -1), 0});

			q.clear();
			q.push_back(src_v);

			// the core of the algorithm
			while(!q.empty()) {
				Index current_v = q.front();
				q.pop_front();

				// check if there is an exit point here
				if(current_v.n_layer == m_nLinks.size() - 1) {
					int flow = m_tLinks.back().edge(current_v.pos).residualCapacity();
					if(flow > 0) {
						path.clear();

						// follow the "parents"
						while(current_v.pos != V2i(-1, -1)) {
							path.add(current_v);

							const Index parent_v = visited.parent(current_v);
							if(parent_v.pos != V2i(-1, -1)) {
								if(parent_v.n_layer == current_v.n_layer) {
									assert(V2i::sqdist(current_v.pos, parent_v.pos) == 1);
									flow = std::min(flow, m_nLinks[current_v.n_layer].edge(parent_v.pos, current_v.pos).residualCapacity());
								}
								else {
									assert(parent_v.n_layer+1 == current_v.n_layer || parent_v.n_layer == current_v.n_layer+1);
									assert(V2i::sqdist(current_v.pos, parent_v.pos) == 0);

									if(parent_v.n_layer < current_v.n_layer)
										flow = std::min(flow, m_tLinks[current_v.n_layer].edge(current_v.pos).residualCapacity());
								}
							}

							current_v = parent_v;
						}

						flow = std::min(flow, m_tLinks.front().edge(path.front().pos).residualCapacity());

						assert(path.isValid());

						offset = src_id+1;

						return flow;
					}
				}

				// try to move horizontally left
				if(current_v.pos.x > 0)
					step(visited, q, current_v, Index{V2i(current_v.pos.x-1, current_v.pos.y), current_v.n_layer});

				// try to move horizontally right
				if(current_v.pos.x < m_size.x-1)
					step(visited, q, current_v, Index{V2i(current_v.pos.x+1, current_v.pos.y), current_v.n_layer});

				// try to move vertically up
				if(current_v.pos.y > 0)
					step(visited, q, current_v, Index{V2i(current_v.pos.x, current_v.pos.y-1), current_v.n_layer});

				// try to move vertically down
				if(current_v.pos.y < m_size.y-1)
					step(visited, q, current_v, Index{V2i(current_v.pos.x, current_v.pos.y+1), current_v.n_layer});

				// try to move down a layer
				if((current_v.n_layer < m_nLinks.size() - 1)) {
					const Index new_v{current_v.pos, current_v.n_layer+1};
					if(!visited.visited(new_v) && (m_tLinks[new_v.n_layer].edge(current_v.pos).residualCapacity() > 0)) {
						visited.visit(new_v, current_v);
						q.push_back(new_v);
					}
				}

				// try to move up a layer (unconditional)
				if((current_v.n_layer > 0)) {
					const Index new_v{current_v.pos, current_v.n_layer-1};
					if(!visited.visited(new_v)) {
						visited.visit(new_v, current_v);
						q.push_back(new_v);
					}
				}
			}
		}
	}

	return 0;
}

int Graph::flow(const Path& path) const {
	assert(path.isValid() && !path.empty());

	assert(path.front().n_layer == 0);
	assert(path.back().n_layer == m_nLinks.size()-1);

	auto it = path.begin();

	// first T-link layer
	int result = m_tLinks.front().edge(it->pos).residualCapacity();

	// middle layers
	Index current = *it;
	++it;

	for(; it != path.end(); ++it) {
		// same layer - need to use N link
		if(current.n_layer == it->n_layer) {
			assert(V2i::sqdist(current.pos, it->pos) == 1);

			result = std::min(result, m_nLinks[current.n_layer].edge(current.pos, it->pos).residualCapacity());

			current.pos = it->pos;
		}

		// different layer - need to use a T link (if the move is to higher layer)
		else if(current.n_layer < it->n_layer) {
			current.n_layer++;
			assert(current.n_layer == it->n_layer);

			result = std::min(result, m_tLinks[current.n_layer].edge(current.pos).residualCapacity());
		}

		// different layer - "infinite capacity" back links have no impact on the flow
		else {
			assert(current.n_layer > 0);
			current.n_layer--;
			assert(current.n_layer == it->n_layer);
		}
	}

	// last T-link layer
	assert(current.n_layer == m_nLinks.size()-1 && "the path should lead to the last layer");
	result = std::min(result, m_tLinks.back().edge(current.pos).residualCapacity());

	return result;
}

void Graph::solve() {
	Path path;

	std::size_t counter = 0;

	std::size_t offset = 0;

	int flow;
	while((flow = bfs_2(path, offset))) {
		assert(!path.empty());
		assert(path.isValid());
		assert(this->flow(path) == flow);

		assert(flow > 0 && "Augmented path flow at the beginning of each iteration should be positive");

		// update the graph starting from first T link layer
		assert(path.front().n_layer == 0);
		m_tLinks.front().edge(path.front().pos).addFlow(flow);

		auto it1 = path.begin();
		auto it2 = it1+1;
		while(it2 != path.end()) {
			// in-layer layer move - use N links
			if(it1->n_layer == it2->n_layer) {
				m_nLinks[it1->n_layer].edge(it1->pos, it2->pos).addFlow(flow);

				assert(m_nLinks[it1->n_layer].edge(it1->pos, it2->pos).residualCapacity() >= 0);
				assert(m_nLinks[it1->n_layer].edge(it2->pos, it1->pos).residualCapacity() >= 0);
			}

			// between-layer move - use T links
			else {
				assert(it1->pos == it2->pos);
				assert(it1->n_layer+1 == it2->n_layer || it1->n_layer == it2->n_layer+1);

				if(it1->n_layer < it2->n_layer)
					m_tLinks[it2->n_layer].edge(it1->pos).addFlow(flow);

				assert(m_tLinks[it2->n_layer].edge(it1->pos).residualCapacity() >= 0);
			}

			++it1;
			++it2;
		}

		// last T link layer
		assert(it1->n_layer == m_tLinks.size()-2);
		m_tLinks.back().edge(it1->pos).addFlow(flow);

		assert(this->flow(path) == 0 && "Augmented path flow at the end of each iteration should be zero");

		++counter;
	}

	std::cout << "ST-cut solve with " << counter << " steps finished." << std::endl;
}

cv::Mat Graph::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result = cv::Mat(m_size.y, m_size.x, CV_8UC1, 255);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			for(std::size_t a=0; a<m_tLinks.size(); ++a)
				if(m_tLinks[a].edge(V2i(x, y)).residualCapacity() > 0)
					result.at<unsigned char>(y, x) = a * (255 / (m_tLinks.size()-1));
		}

	// // reachable from source as a trivial DFS search
	// std::vector<Index> stack;
	// for(int y=0; y<m_size.y; ++y)
	// 	for(int x=0; x<m_size.x; ++x)
	// 		if(m_tLinks.front().edge(V2i(x, y)).residualCapacity() > 0) {
	// 			// std::cout << x << "," << y << "  ";
	// 			stack.push_back(Index{V2i(x, y), 0});

	// 			std::vector<bool> visited(m_size.x * m_size.y * m_nLinks.size());

	// 			while(!stack.empty()) {
	// 				const Index current = stack.back();
	// 				stack.pop_back();

	// 				assert(m_tLinks.back().edge(current.pos).residualCapacity() == 0 && "None of the paths should have a direct link from source to the sink without turning on an N link.");

	// 				unsigned char& value = result.at<unsigned char>(current.pos.y, current.pos.x);
	// 				const int target = (current.n_layer) * (255 / m_nLinks.size());
	// 				// std::cout << target << " ";

	// 				if(!visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y]) {
	// 					visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y] = true;

	// 				// if(target != value) {
	// 					// std::cout << "#";

	// 					if(target > value || value == 255)
	// 						value = target;

	// 					if(current.n_layer < m_nLinks.size()) {
	// 						// horizontal move
	// 						if(current.pos.x > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x-1, current.pos.y)).residualCapacity() > 0)
	// 							stack.push_back(Index{V2i(current.pos.x-1, current.pos.y), current.n_layer});
	// 						if(current.pos.x < m_size.x-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x+1, current.pos.y)).residualCapacity() > 0)
	// 							stack.push_back(Index{V2i(current.pos.x+1, current.pos.y), current.n_layer});

	// 						// vertical move
	// 						if(current.pos.y > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y-1)).residualCapacity() > 0)
	// 							stack.push_back(Index{V2i(current.pos.x, current.pos.y-1), current.n_layer});
	// 						if(current.pos.y < m_size.y-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y+1)).residualCapacity() > 0)
	// 							stack.push_back(Index{V2i(current.pos.x, current.pos.y+1), current.n_layer});

	// 						// layer move down
	// 						assert(current.n_layer < m_nLinks.size());
	// 						if(m_tLinks[current.n_layer+1].edge(current.pos).residualCapacity() > 0)
	// 							stack.push_back(Index{current.pos, current.n_layer+1});

	// 						// // layer move up (unconditional)
	// 						// if(current.n_layer > 0)
	// 						// 	stack.push_back(Index{current.pos, current.n_layer-1});
	// 					}
	// 				}
	// 			}

	// 			// std::cout << std::endl;
	// 		}

	return result;
}

std::vector<cv::Mat> Graph::debug() const {
	assert(m_nLinks.size() + 1 == m_tLinks.size());

	std::vector<cv::Mat> result;
	for(std::size_t a=0;a<m_nLinks.size(); ++a) {
		result.push_back(t_flow(m_tLinks[a]));

		result.push_back(n_flow_horiz(m_nLinks[a]));
		result.push_back(n_flow_vert(m_nLinks[a]));
	}

	result.push_back(t_flow(m_tLinks.back()));

	return result;
}

cv::Mat Graph::t_flow(const TLinks& t) const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = t.edge(V2i(x, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::n_flow_horiz(const NLinks& t) const {
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x-1; ++x) {
			auto& e = t.edge(V2i(x, y), V2i(x+1, y));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

cv::Mat Graph::n_flow_vert(const NLinks& t) const {
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y-1; ++y)
		for(int x=0; x<m_size.x; ++x) {
			auto& e = t.edge(V2i(x, y), V2i(x, y+1));
			if(e.capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.flow()) * 255 / e.capacity();
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

}
