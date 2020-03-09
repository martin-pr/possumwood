#include "graph.h"

#include <cassert>
#include <map>
#include <queue>

#include "bfs_visitors.h"

namespace lightfields {

Graph::Graph(const V2i& size, int n_link_value, std::size_t layer_count) : m_size(size) {
	m_tLinks = std::vector<TLinks>(layer_count, TLinks(size));
	m_nLinks = std::vector<NLinks>(layer_count, NLinks(size, n_link_value));
}

void Graph::setValue(const V2i& pos, const std::vector<int>& values) {
	assert(values.size() == m_tLinks.size());

	auto tit = m_tLinks.begin();
	auto vit = values.begin();

	// int min = values.front();

	while(tit != m_tLinks.end()) {
		tit->edge(pos).setCapacity(*vit);

		assert(*vit >= 0);
		assert(tit->edge(pos).residualCapacity() == *vit);

		// min = std::min(min, *vit);

		++tit;
		++vit;
	}

	// for(tit = m_tLinks.begin(); tit != m_tLinks.end(); ++tit)
	// 	tit->edge(pos).addFlow(min);
}

std::size_t Graph::v2i(const V2i& v) const {
	return v.x + v.y*m_size.x;
}

V2i Graph::i2v(std::size_t v) const {
	v = v % (m_size.x*m_size.y);
	return V2i(v % m_size.x, v / m_size.x);
}

namespace {

struct QueueItem {
	Index current, parent;
};

}

int Graph::bfs_2(GraphPath& path, std::size_t& offset) const {
	path.clear();

	BFSVisitors visited(m_size, m_nLinks.size());
	std::deque<QueueItem> q;

	const std::size_t end = m_size.x * m_size.y;
	for(std::size_t i=0; i<end; ++i) {
		const std::size_t src_id = (i + offset) % end;
		const Index src_v = Index{i2v(src_id), 0};

		// initialize - source index has parent -1,-1
		visited.clear();

		q.clear();
		q.push_back(QueueItem{src_v, Index{V2i(-1, -1), 0}});

		// the core of the algorithm
		while(!q.empty()) {
			QueueItem item = q.front();
			q.pop_front();

			if(!visited.visited(item.current)) {
				visited.visit(item.current, item.parent);

				Index current_v = item.current;

				// try to move horizontally left
				if(current_v.pos.x > 0) {
					const Index new_v{V2i(current_v.pos.x-1, current_v.pos.y), current_v.n_layer};
					if(m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0)
						q.push_back(QueueItem{new_v, current_v});
				}

				// try to move horizontally right
				if(current_v.pos.x < m_size.x-1){
					const Index new_v{V2i(current_v.pos.x+1, current_v.pos.y), current_v.n_layer};
					if(m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0)
						q.push_back(QueueItem{new_v, current_v});
				}

				// try to move vertically up
				if(current_v.pos.y > 0) {
					const Index new_v{V2i(current_v.pos.x, current_v.pos.y-1), current_v.n_layer};
					if(m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0)
						q.push_back(QueueItem{new_v, current_v});
				}

				// try to move vertically down
				if(current_v.pos.y < m_size.y-1) {
					const Index new_v{V2i(current_v.pos.x, current_v.pos.y+1), current_v.n_layer};
					if(m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0)
						q.push_back(QueueItem{new_v, current_v});
				}

				// try to move up a layer (unconditional)
				if((current_v.n_layer > 0)) {
					const Index new_v{current_v.pos, current_v.n_layer-1};
					q.push_back(QueueItem{new_v, current_v});
				}

				// try to move down a layer
				if((current_v.n_layer < m_nLinks.size()-1)) {
					const Index new_v{current_v.pos, current_v.n_layer+1};
					if(m_tLinks[current_v.n_layer].edge(current_v.pos).residualCapacity() > 0)
						q.push_back(QueueItem{new_v, current_v});
				}

				// the potential exit point
				if(current_v.n_layer == m_nLinks.size()-1) {
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
										flow = std::min(flow, m_tLinks[parent_v.n_layer].edge(parent_v.pos).residualCapacity());
								}
							}

							current_v = parent_v;
						}

						assert(path.isValid());

						offset = src_id+1;

						assert(flow > 0);
						return flow;
					}
				}
			}
		}
	}

	return 0;
}

int Graph::flow(const GraphPath& path) const {
	assert(path.isValid() && !path.empty());

	assert(path.front().n_layer == 0);
	assert(path.back().n_layer == m_nLinks.size()-1);

	// initial flow value
	int result = std::numeric_limits<int>::max();

	// iterate over all layers
	auto it = path.begin();
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
			assert(current.n_layer+1 == it->n_layer);
			assert(current.pos == it->pos);

			result = std::min(result, m_tLinks[current.n_layer].edge(current.pos).residualCapacity());

			current.n_layer = it->n_layer;
		}

		// different layer - "infinite capacity" back links have no impact on the flow
		else {
			assert(current.n_layer > 0);
			assert(current.n_layer - it->n_layer == 1);

			current.n_layer--;

			assert(current.n_layer == it->n_layer);
		}
	}

	// last T-link layer
	assert(current.n_layer == m_nLinks.size()-1 && "the path should lead to the last layer");
	result = std::min(result, m_tLinks.back().edge(current.pos).residualCapacity());

	return result;
}

void Graph::doFlow(const GraphPath& path, int flow) {
	assert(!path.empty());
	assert(path.isValid());

	assert(this->flow(path) == flow);

	assert(flow > 0 && "Augmented path flow at the beginning of each iteration should be positive");

	// update the graph starting from first T link layer
	assert(path.front().n_layer == 0);

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
				m_tLinks[it1->n_layer].edge(it1->pos).addFlow(flow);

			assert(m_tLinks[it2->n_layer].edge(it1->pos).residualCapacity() >= 0);
		}

		++it1;
		++it2;
	}

	// last T link layer
	assert(it1->n_layer == m_tLinks.size()-1);
	m_tLinks.back().edge(it1->pos).addFlow(flow);

	assert(this->flow(path) == 0 && "Augmented path flow at the end of each iteration should be zero");
}

void Graph::solve() {
	GraphPath path;

	std::size_t counter = 0;
	std::size_t offset = 0;

	int flow;
	while((flow = bfs_2(path, offset))) {
		doFlow(path, flow);

		++counter;
	}

	std::cout << "ST-cut solve with " << counter << " steps finished." << std::endl;
}

cv::Mat Graph::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	// for(int y=0; y<m_size.y; ++y)
	// 	for(int x=0; x<m_size.x; ++x) {
	// 		// int a=0;
	// 		for(std::size_t a=0; a<m_tLinks.size(); ++a)
	// 			if(m_tLinks[a].edge(V2i(x, y)).residualCapacity() == 0)
	// 				result.at<unsigned char>(y, x) = a * (255 / (m_tLinks.size() - 1));
	// 	}

	// reachable from source as a trivial DFS search
	{
		std::vector<Index> stack;
		for(int y=0; y<m_size.y; ++y)
			for(int x=0; x<m_size.x; ++x) {
				// if(m_tLinks.front().edge(V2i(x, y)).residualCapacity() > 0) {
					// std::cout << x << "," << y << "  ";
					stack.push_back(Index{V2i(x, y), 0});
					std::vector<bool> visited(m_size.x * m_size.y * (m_nLinks.size()+1));

					while(!stack.empty()) {
						const Index current = stack.back();
						stack.pop_back();

						// assert(m_tLinks.back().edge(current.pos).residualCapacity() == 0 && "None of the paths should have a direct link from source to the sink without turning on an N link.");

						unsigned char& value = result.at<unsigned char>(current.pos.y, current.pos.x);
						const int target = (current.n_layer) * (255 / m_nLinks.size());
						// std::cout << target << " ";

						if(!visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y]) {
							visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y] = true;

						// if(target != value) {
							// std::cout << "#";

							if(target > value)
								value = target;

							if(current.n_layer < m_nLinks.size()) {
								// horizontal move
								if(current.pos.x > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x-1, current.pos.y)).residualCapacity() > 0)
									stack.push_back(Index{V2i(current.pos.x-1, current.pos.y), current.n_layer});
								if(current.pos.x < m_size.x-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x+1, current.pos.y)).residualCapacity() > 0)
									stack.push_back(Index{V2i(current.pos.x+1, current.pos.y), current.n_layer});

								// vertical move
								if(current.pos.y > 0 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y-1)).residualCapacity() > 0)
									stack.push_back(Index{V2i(current.pos.x, current.pos.y-1), current.n_layer});
								if(current.pos.y < m_size.y-1 && m_nLinks[current.n_layer].edge(current.pos, V2i(current.pos.x, current.pos.y+1)).residualCapacity() > 0)
									stack.push_back(Index{V2i(current.pos.x, current.pos.y+1), current.n_layer});

								// layer move down
								assert(current.n_layer < m_nLinks.size());
								if(m_tLinks[current.n_layer].edge(current.pos).residualCapacity() > 0)
									stack.push_back(Index{current.pos, current.n_layer+1});

								// layer move up (unconditional)
								if(current.n_layer > 0)
									stack.push_back(Index{current.pos, current.n_layer-1});
							}
						}
					}
			}
	}

	return result;
}

std::vector<cv::Mat> Graph::debug() const {
	std::vector<cv::Mat> result;
	for(std::size_t a=0;a<m_nLinks.size(); ++a) {
		result.push_back(n_flow(m_nLinks[a]));
		result.push_back(t_flow(m_tLinks[a]));
	}

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

cv::Mat Graph::n_flow(const NLinks& n) const {
	cv::Mat result(m_size.y, m_size.x, CV_8UC1);

	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			float residual = 0.0f;
			float max = 0.0f;
			if(x > 0) {
				auto& e = n.edge(V2i(x-1, y), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(x < m_size.x-1) {
				auto& e = n.edge(V2i(x+1, y), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(y > 0) {
				auto& e = n.edge(V2i(x, y-1), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(y < m_size.y-1) {
				auto& e = n.edge(V2i(x, y+1), V2i(x, y));
				if(e.capacity() > 0) {
					residual += (float)e.residualCapacity();
					max += (float)e.capacity();
				}
			}

			if(max > 0.0f)
				result.at<unsigned char>(y,x) = 255.0f - residual / max * 255.0f;
			else
				result.at<unsigned char>(y,x) = 255;
		}

	return result;
}

}
