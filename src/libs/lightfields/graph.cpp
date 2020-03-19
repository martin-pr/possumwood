#include "graph.h"

#include <cassert>
#include <map>
#include <queue>
#include <functional>

#include <tbb/task_group.h>

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

	int minFlow = values.front();

	while(tit != m_tLinks.end()) {
		tit->edge(pos).setCapacity(*vit, std::numeric_limits<int>::max()/2);

		assert(*vit >= 0);
		assert(tit->edge(pos).forward().residualCapacity() == *vit);
		assert(tit->edge(pos).backward().residualCapacity() == std::numeric_limits<int>::max()/2);

		// tit->edge(pos).setCapacity(*vit, *vit);

		// assert(*vit >= 0);
		// assert(tit->edge(pos).forward().residualCapacity() == *vit);
		// assert(tit->edge(pos).backward().residualCapacity() == *vit);

		minFlow = std::min(minFlow, *vit);

		++tit;
		++vit;
	}

	// // speedup of the iterations - addresses the "trivial cases" removing the direct column
	// // flow from source to sink
	// if(minFlow > 0)
	// 	for(tit = m_tLinks.begin(); tit != m_tLinks.end(); ++tit)
	// 		tit->edge(pos).forward().addFlow(minFlow);
}

std::size_t Graph::v2i(const V2i& v) const {
	return v.x + v.y*m_size.x;
}

namespace {

struct QueueItem {
	Index current, parent;
};

}

bool Graph::iterate(const tbb::blocked_range2d<int>& range) {
	bool foundPath = false;

	BFSVisitors visited(range, m_nLinks.size());
	std::deque<QueueItem> q;

	// initialize - source index has parent -1,-1
	for(int y=range.rows().begin(); y != range.rows().end(); ++y)
		for(int x=range.cols().begin(); x != range.cols().end(); ++x) {
			const Index src_v{V2i(x, y), 0};

			visited.visit(src_v, Index{V2i(-1, -1), 0});
			q.push_back(QueueItem{src_v, Index{V2i(-1, -1), 0}});
		}

	// the core of the algorithm
	while(!q.empty()) {
		QueueItem item = q.front();
		q.pop_front();

		Index current_v = item.current;

		// a potential exit point
		if(current_v.n_layer == m_nLinks.size()-1) {
			if(m_tLinks.back().edge(current_v.pos).forward().residualCapacity() > 0) {
				foundPath = true;

				const int f = flow(visited, current_v);
				if(f > 0)
					doFlow(visited, current_v, f);
			}
		}

		// try to move horizontally left
		if(current_v.pos.x > range.cols().begin()) {
			const Index new_v{V2i(current_v.pos.x-1, current_v.pos.y), current_v.n_layer};
			if(!visited.visited(new_v) && m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move horizontally right
		if(current_v.pos.x < range.cols().end()-1){
			const Index new_v{V2i(current_v.pos.x+1, current_v.pos.y), current_v.n_layer};
			if(!visited.visited(new_v) && m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move vertically up
		if(current_v.pos.y > range.rows().begin()) {
			const Index new_v{V2i(current_v.pos.x, current_v.pos.y-1), current_v.n_layer};
			if(!visited.visited(new_v) && m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move vertically down
		if(current_v.pos.y < range.rows().end()-1) {
			const Index new_v{V2i(current_v.pos.x, current_v.pos.y+1), current_v.n_layer};
			if(!visited.visited(new_v) && m_nLinks[current_v.n_layer].edge(current_v.pos, new_v.pos).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move up a layer
		if((current_v.n_layer > 0)) {
			const Index new_v{current_v.pos, current_v.n_layer-1};
			if(!visited.visited(new_v) && m_tLinks[new_v.n_layer].edge(new_v.pos).backward().residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move down a layer
		if((current_v.n_layer < m_nLinks.size()-1)) {
			const Index new_v{current_v.pos, current_v.n_layer+1};
			if(!visited.visited(new_v) && m_tLinks[current_v.n_layer].edge(current_v.pos).forward().residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}
	}

	return foundPath;
}

int Graph::flow(const BFSVisitors& visitors, const Index& end) const {
	assert(end.pos.x != -1 && end.pos.y != -1);
	assert(end.n_layer == m_nLinks.size()-1);

	// initial flow value based on the last T link (towards the sink)
	int result = m_tLinks.back().edge(end.pos).forward().residualCapacity();

	Index current = end;
	Index parent = visitors.parent(current);

	while(result > 0 && parent.pos.x != -1 && parent.pos.y != -1) {
		assert(Index::sqdist(current, parent) == 1);

		// same layer - need to use N link
		if(current.n_layer == parent.n_layer)
			result = std::min(result, m_nLinks[current.n_layer].edge(parent.pos, current.pos).residualCapacity());

		// different layer down - need to use a T link (if the move is to higher layer)
		else if(current.n_layer > parent.n_layer)
			result = std::min(result, m_tLinks[parent.n_layer].edge(parent.pos).forward().residualCapacity());

		// different layer up
		else if(current.n_layer < parent.n_layer)
			result = std::min(result, m_tLinks[current.n_layer].edge(current.pos).backward().residualCapacity());

		// and move on
		current = parent;
		parent = visitors.parent(current);
	}

	assert(current.n_layer == 0 || result == 0);

	return result;
}

void Graph::doFlow(const BFSVisitors& visitors, const Index& end, int flow) {
	assert(end.pos.x != -1 && end.pos.y != -1);
	assert(end.n_layer == m_nLinks.size()-1);

	// initial flow value based on the last T link (towards the sink)
	m_tLinks.back().edge(end.pos).forward().addFlow(flow);


	Index current = end;
	Index parent = visitors.parent(current);

	while(parent.pos.x != -1 && parent.pos.y != -1) {
		assert(Index::sqdist(current, parent) == 1);

		// same layer - need to use N link
		if(current.n_layer == parent.n_layer)
			m_nLinks[current.n_layer].edge(parent.pos, current.pos).addFlow(flow);

		// different layer down - need to use a T link (if the move is to higher layer)
		else if(current.n_layer > parent.n_layer)
			m_tLinks[parent.n_layer].edge(parent.pos).forward().addFlow(flow);

		// different layer up
		else if(current.n_layer < parent.n_layer)
			m_tLinks[current.n_layer].edge(current.pos).backward().addFlow(flow);

		// and move on
		current = parent;
		parent = visitors.parent(current);
	}

	assert(current.n_layer == 0);
}

void Graph::edmondsKarpSolve() {
	std::size_t counter = 0;

	tbb::task_group tasks;

	// 23/2=11/2=5/2=2/2=1 - on each step produces a different division line,
	// but ends with the "largest possible" subdivision of 2x2 before descending into
	// single-threaded finishing
	int div = 23;

	// an interative function that can spawn itself if the iteration didn't finish
	std::function<void(const tbb::blocked_range2d<int>& range)> fn;
	fn = [&](const tbb::blocked_range2d<int>& range) {
		if(iterate(range)) {
			tasks.run([range, &fn]() {
				fn(range);
			});
		}

		++counter;
	};

	// iterate on each subdiv level
	while(div > 0) {
		for(int y=0;y<div;++y)
			for(int x=0;x<div;++x) {
				const tbb::blocked_range2d<int> range(
					(m_size.y * y) / div, (m_size.y * (y+1)) / div,
					(m_size.x * x) / div, (m_size.x * (x+1)) / div
				);

				tasks.run([range, &fn]() {
					fn(range);
				});
			}

		// execute tasks and wait for each level to finish
		tasks.wait();

		div /= 2;
	}

	std::cout << "ST-cut solve with " << counter << " iterations finished." << std::endl;
}

Index Graph::val2index(std::size_t value) const {
	Index result;

	result.n_layer = value / std::size_t(m_size.x * m_size.y);
	value %= (m_size.x * m_size.y);

	result.pos = V2i(value % m_size.x, value / m_size.x);

	return result;
}

std::size_t Graph::index2val(const Index& i) const {
	std::size_t result = i.pos.x + i.pos.y * m_size.x;
	result += std::size_t(i.n_layer) * std::size_t(m_size.x * m_size.y);

	return result;
}

bool Graph::pushHoriz(const Index& current, const Index& next, const std::vector<int>& label, std::vector<int>& excess) {
	assert(V2i::sqdist(current.pos, next.pos) == 1);
	assert(current.n_layer == next.n_layer);
	assert(current.n_layer < m_nLinks.size());

	const std::size_t current_v = index2val(current);
	const std::size_t next_v = index2val(next);

	if(label[current_v] == label[next_v] + 1) {
		auto& edge = m_nLinks[current.n_layer].edge(current.pos, next.pos);

		const int flow = std::min(edge.residualCapacity(), excess[current_v]);
		if(flow > 0) {
			edge.addFlow(flow);
			excess[current_v] -= flow;
			excess[next_v] += flow;

			return true;
		}
	}

	return false;
}

bool Graph::pushVertDown(const Index& current, const std::vector<int>& label, std::vector<int>& excess) {
	assert(current.n_layer < m_tLinks.size());

	const std::size_t current_v = index2val(current);
	const std::size_t next_v = index2val(Index{current.pos, current.n_layer+1});

	assert(current_v < label.size());

	if((label[current_v] == 1 && current.n_layer+1 == m_nLinks.size()) || label[current_v] == label[next_v] + 1) {
		auto& edge = m_tLinks[current.n_layer].edge(current.pos);

		const int flow = std::min(edge.forward().residualCapacity(), excess[current_v]);
		if(flow > 0) {
			edge.forward().addFlow(flow);
			excess[current_v] -= flow;
			if(next_v < excess.size())
				excess[next_v] += flow;

			return true;
		}
	}

	return false;
}

bool Graph::pushVertUp(const Index& current, const std::vector<int>& label, std::vector<int>& excess) {
	assert(current.n_layer < m_tLinks.size());

	const std::size_t current_v = index2val(current);

	// flowing "back to the source" - no explicit edge, just flow everything back
	if(current.n_layer == 0 && label[current_v] == m_size.x * m_size.y + 1) {
		excess[current_v] = 0;
		return true;
	}

	// flow between layers with explicit edges
	if(current.n_layer > 0) {
		const Index next{current.pos, current.n_layer-1};
		const std::size_t next_v = index2val(next);

		assert(current_v < label.size());
		// assert(next_v < label.size());

		if(label[current_v] == label[next_v] + 1) {
			auto& edge = m_tLinks[next.n_layer].edge(next.pos);

			const int flow = std::min(edge.backward().residualCapacity(), excess[current_v]);
			if(flow > 0) {
				edge.backward().addFlow(flow);
				excess[current_v] -= flow;

				assert(next_v < excess.size());
				excess[next_v] += flow;

				return true;
			}
		}
	}

	return false;
}

bool Graph::relabel(const Index& current, std::vector<int>& labels, const std::vector<int>& excess) const {
	const std::size_t current_v = index2val(current);
	assert(excess[current_v] > 0);

	int label = std::numeric_limits<int>::max();

	if(current.pos.x > 0) {
		const V2i target(current.pos.x-1, current.pos.y);

		auto& e = m_nLinks[current.n_layer].edge(current.pos, target);
		if(e.residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{target, current.n_layer});
			label = std::min(label, labels[target_v]);
		}
	}

	if(current.pos.x < m_size.x - 1) {
		const V2i target(current.pos.x+1, current.pos.y);

		auto& e = m_nLinks[current.n_layer].edge(current.pos, target);
		if(e.residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{target, current.n_layer});
			label = std::min(label, labels[target_v]);
		}
	}

	if(current.pos.y > 0) {
		const V2i target(current.pos.x, current.pos.y-1);

		auto& e = m_nLinks[current.n_layer].edge(current.pos, target);
		if(e.residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{target, current.n_layer});
			label = std::min(label, labels[target_v]);
		}
	}

	if(current.pos.y < m_size.y - 1) {
		const V2i target(current.pos.x, current.pos.y+1);

		auto& e = m_nLinks[current.n_layer].edge(current.pos, target);
		if(e.residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{target, current.n_layer});
			label = std::min(label, labels[target_v]);
		}
	}

	{
		assert(current.n_layer < m_nLinks.size());

		const V2i target(current.pos.x, current.pos.y);

		auto& e = m_tLinks[current.n_layer].edge(current.pos);
		if(e.forward().residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{target, current.n_layer+1});
			if(target_v < labels.size())
				label = std::min(label, labels[target_v]);
			else
				label = 0;
		}
	}

	if(current.n_layer == 0) {
		label = std::min(label, m_size.x * m_size.y);
	}
	else {
		auto& e = m_tLinks[current.n_layer-1].edge(current.pos);
		if(e.backward().residualCapacity() > 0) {
			const std::size_t target_v = index2val(Index{current.pos, current.n_layer-1});

			label = std::min(label, labels[target_v]);
		}
	}

	if(label < std::numeric_limits<int>::max() && label+1 != labels[current_v] ) {
		labels[current_v] = label+1;
		return true;
	}

	return false;
}

void Graph::pushRelabelSolve() {
	assert(m_nLinks.size() > 1);

	std::vector<int> label(m_size.x * m_size.y * m_nLinks.size());
	std::vector<int> excess(m_size.x * m_size.y * m_nLinks.size());

	// first of all, push from source - infinite capacity means maximum excess for each cell bordering with source
	for(int a=0; a<m_size.x * m_size.y; ++a)
		excess[a] = std::numeric_limits<int>::max() / 4;

	std::size_t activeCount = 1;

	while(activeCount > 0) {
		activeCount = 0;

		// // first of all, push from source - infinite capacity means maximum excess for each cell bordering with source
		// for(int a=0; a<m_size.x * m_size.y; ++a)
		// 	if(label[a] < m_size.x * m_size.y && excess[a] > 0)
		// 		excess[a] = std::numeric_limits<int>::max() / 4;

		// then push from all active cells
		for(std::size_t current_v=0; current_v<excess.size(); ++current_v) {
			const Index current_i = val2index(current_v);
			assert(index2val(current_i) == current_v);

			// if(excess[current_v] > 0)
			// 	std::cout << "cell " << current_i << "/" << m_tLinks.size() << std::endl;

			// active cell
			bool pushed = true;
			while(excess[current_v] > 0 && pushed) {
				++activeCount;

				// try to push all directions
				pushed = false;
				assert(excess[current_v] > 0);

				if(excess[current_v] > 0 && current_i.pos.x > 0) {
					const Index next_i {V2i(current_i.pos.x-1, current_i.pos.y), current_i.n_layer};
					if(pushHoriz(current_i, next_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push horiz x-1";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0 && current_i.pos.x < m_size.x-1) {
					const Index next_i {V2i(current_i.pos.x+1, current_i.pos.y), current_i.n_layer};
					if(pushHoriz(current_i, next_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push horiz x+1";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0 && current_i.pos.y > 0) {
					const Index next_i {V2i(current_i.pos.x, current_i.pos.y-1), current_i.n_layer};
					if(pushHoriz(current_i, next_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push horiz y-1";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0 && current_i.pos.y < m_size.y-1) {
					const Index next_i {V2i(current_i.pos.x, current_i.pos.y+1), current_i.n_layer};
					if(pushHoriz(current_i, next_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push horiz y+1";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0) {
					if(pushVertDown(current_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push vert down";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0) {
					if(pushVertUp(current_i, label, excess)) {
						pushed = true;
						// std::cout << "  - push vert up";
					}
					else
						assert(excess[current_v] > 0);
				}

				if(excess[current_v] > 0 && !pushed) {
					if(relabel(current_i, label, excess)) {
						pushed = true;
						// std::cout << "  - relabel";
					}
				}

				// std::cout << "  ->  label = " << label[current_v] << ", excess = " << excess[current_v] << std::endl;
			}
		}

		// first row should immediately become inactive
		// for(std::size_t current_v = 0; current_v < std::size_t(m_size.x*m_size.y); ++current_v)
		// 	assert(excess[current_v] == 0);

		std::cout << "active count = " << activeCount << std::endl;
	}

}

cv::Mat Graph::minCut() const {
	// collects all reachable nodes from source or sink, and marks them appropriately
	cv::Mat result = cv::Mat::zeros(m_size.y, m_size.x, CV_8UC1);

	// reachable from source as a trivial DFS search
	std::vector<bool> visited(m_size.x * m_size.y * (m_nLinks.size()+1));

	std::vector<Index> stack;
	for(int y=0; y<m_size.y; ++y)
		for(int x=0; x<m_size.x; ++x) {
			stack.push_back(Index{V2i(x, y), 0});

			while(!stack.empty()) {
				const Index current = stack.back();
				stack.pop_back();

				unsigned char& value = result.at<unsigned char>(current.pos.y, current.pos.x);
				const int target = (current.n_layer) * (255 / (m_nLinks.size() - 1));

				if(!visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y]) {
					visited[v2i(current.pos) + current.n_layer * m_size.x * m_size.y] = true;

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
						if(m_tLinks[current.n_layer].edge(current.pos).forward().residualCapacity() > 0)
							stack.push_back(Index{current.pos, current.n_layer+1});

						// layer move up (unconditional)
						if(current.n_layer > 0 && m_tLinks[current.n_layer].edge(current.pos).backward().residualCapacity() > 0)
							stack.push_back(Index{current.pos, current.n_layer-1});
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
			if(e.forward().capacity() > 0)
				result.at<unsigned char>(y,x) = std::abs(e.forward().flow()) * 255 / e.forward().capacity();
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
