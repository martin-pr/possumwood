#include "graph.h"

#include <cassert>
#include <map>
#include <queue>
#include <functional>
#include <thread>
#include <chrono>

#include <tbb/task_group.h>

#include "bfs_visitors.h"
#include "labels.h"

namespace lightfields {

namespace {

struct QueueItem {
	Index current, parent;
};

Index val2index(std::size_t value, const V2i& size) {
	Index result;

	result.n_layer = value / std::size_t(size.x * size.y);
	value %= (size.x * size.y);

	result.pos = V2i(value % size.x, value / size.x);

	return result;
}

std::size_t index2val(const Index& i, const V2i& size) {
	std::size_t result = i.pos.x + i.pos.y * size.x;
	result += std::size_t(i.n_layer) * std::size_t(size.x * size.y);

	return result;
}

}

bool Graph::iterate(Grid& grid, const tbb::blocked_range2d<int>& range) {
	bool foundPath = false;

	BFSVisitors visited(range, grid.layerCount());
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
		if(current_v.n_layer == grid.layerCount()-1) {
			if(grid.edge(current_v, Index{current_v.pos, current_v.n_layer+1}).residualCapacity() > 0) {
				foundPath = true;

				const int f = flow(grid, visited, current_v);
				if(f > 0)
					doFlow(grid, visited, current_v, f);
			}
		}

		// try to move horizontally left
		if(current_v.pos.x > range.cols().begin()) {
			const Index new_v{V2i(current_v.pos.x-1, current_v.pos.y), current_v.n_layer};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move horizontally right
		if(current_v.pos.x < range.cols().end()-1){
			const Index new_v{V2i(current_v.pos.x+1, current_v.pos.y), current_v.n_layer};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move vertically up
		if(current_v.pos.y > range.rows().begin()) {
			const Index new_v{V2i(current_v.pos.x, current_v.pos.y-1), current_v.n_layer};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move vertically down
		if(current_v.pos.y < range.rows().end()-1) {
			const Index new_v{V2i(current_v.pos.x, current_v.pos.y+1), current_v.n_layer};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move up a layer
		if((current_v.n_layer > 0)) {
			const Index new_v{current_v.pos, current_v.n_layer-1};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}

		// try to move down a layer
		if((current_v.n_layer < grid.layerCount()-1)) {
			const Index new_v{current_v.pos, current_v.n_layer+1};
			if(!visited.visited(new_v) && grid.edge(current_v, new_v).residualCapacity() > 0) {
				visited.visit(new_v, current_v);
				q.push_back(QueueItem{new_v, current_v});
			}
		}
	}

	return foundPath;
}

int Graph::flow(Grid& grid, const BFSVisitors& visitors, const Index& end) const {
	assert(end.pos.x != -1 && end.pos.y != -1);
	assert(end.n_layer == grid.layerCount()-1);

	// initial flow value based on the last T link (towards the sink)
	int result = grid.edge(end, Index{end.pos, end.n_layer+1}).residualCapacity();

	Index current = end;
	Index parent = visitors.parent(current);

	while(result > 0 && parent.pos.x != -1 && parent.pos.y != -1) {
		assert(Index::sqdist(current, parent) == 1);

		// try to find the minimum capacity
		result = std::min(result, grid.edge(parent, current).residualCapacity());

		// and move on
		current = parent;
		parent = visitors.parent(current);
	}

	assert(current.n_layer == 0 || result == 0);

	return result;
}

void Graph::doFlow(Grid& grid, const BFSVisitors& visitors, const Index& end, int flow) {
	assert(end.pos.x != -1 && end.pos.y != -1);
	assert(end.n_layer == grid.layerCount()-1);

	// initial flow value based on the last T link (towards the sink)
	grid.edge(end, Index{end.pos, end.n_layer+1}).addFlow(flow);

	Index current = end;
	Index parent = visitors.parent(current);

	while(parent.pos.x != -1 && parent.pos.y != -1) {
		assert(Index::sqdist(current, parent) == 1);

		// propagate
		grid.edge(parent, current).addFlow(flow);

		// and move on
		current = parent;
		parent = visitors.parent(current);
	}

	assert(current.n_layer == 0);
}

void Graph::edmondsKarpSolve(Grid& grid) {
	std::size_t counter = 0;

	tbb::task_group tasks;

	// 23/2=11/2=5/2=2/2=1 - on each step produces a different division line,
	// but ends with the "largest possible" subdivision of 2x2 before descending into
	// single-threaded finishing
	int div = 23;

	// an interative function that can spawn itself if the iteration didn't finish
	std::function<void(const tbb::blocked_range2d<int>& range)> fn;
	fn = [&](const tbb::blocked_range2d<int>& range) {
		if(iterate(grid, range)) {
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
					(grid.size().y * y) / div, (grid.size().y * (y+1)) / div,
					(grid.size().x * x) / div, (grid.size().x * (x+1)) / div
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

bool Graph::pushHoriz(Grid& grid, const Index& current, const Index& next, const Labels& label, int& excess, ActiveQueue& queue) {
	assert(V2i::sqdist(current.pos, next.pos) == 1);
	assert(current.n_layer == next.n_layer);
	assert(current.n_layer < grid.layerCount());

	const std::size_t current_v = index2val(current, grid.size());
	const std::size_t next_v = index2val(next, grid.size());

	if(label[current_v] == label[next_v] + 1) {
		auto& edge = grid.edge(current, next);

		const int flow = std::min(edge.residualCapacity(), excess);
		if(flow > 0) {
			edge.addFlow(flow);
			excess -= flow;
			queue.push(ActiveQueue::Item(next_v, flow, label[next_v]));

			return true;
		}
	}

	return false;
}

bool Graph::pushVertDown(Grid& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue) {
	assert(current.n_layer < grid.layerCount());

	const Index next {current.pos, current.n_layer+1};
	const std::size_t current_v = index2val(current, grid.size());
	const std::size_t next_v = index2val(next, grid.size());

	assert(current_v < label.size());

	if((label[current_v] == 1 && current.n_layer+1 == grid.layerCount()) || (next_v < label.size() && label[current_v] == label[next_v] + 1)) {
		auto& edge = grid.edge(current, next);

		const int flow = std::min(edge.residualCapacity(), excess);
		if(flow > 0) {
			edge.addFlow(flow);
			excess -= flow;
			if(next_v < label.size())
				queue.push(ActiveQueue::Item(next_v, flow, label[next_v]));

			return true;
		}
	}

	return false;
}

bool Graph::pushVertUp(Grid& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue) {
	assert(current.n_layer < grid.layerCount());

	const std::size_t current_v = index2val(current, grid.size());

	// flowing "back to the source" - no explicit edge, just flow everything back
	if(current.n_layer == 0 && label[current_v] == unsigned(grid.size().x * grid.size().y + 1)) {
		excess = 0;
		return true;
	}

	// flow between layers with explicit edges
	if(current.n_layer > 0) {
		const Index next{current.pos, current.n_layer-1};
		const std::size_t next_v = index2val(next, grid.size());

		assert(current_v < label.size());
		// assert(next_v < label.size());

		if(label[current_v] == label[next_v] + 1) {
			auto& edge = grid.edge(current, next);

			const int flow = std::min(edge.residualCapacity(), excess);
			if(flow > 0) {
				edge.addFlow(flow);
				excess -= flow;
				assert(next_v < label.size());
				queue.push(ActiveQueue::Item{next_v, flow, label[next_v]});

				return true;
			}
		}
	}

	return false;
}

namespace {

void updateLabel(unsigned& label, const Index& current, const Index& target, const Grid& grid, const Labels& labels) {
	auto& e = grid.edge(current, target);
	if(e.residualCapacity() > 0) {
		const std::size_t target_v = index2val(target, grid.size());
		label = std::min(label, unsigned(labels[target_v]));
	}
}

}

bool Graph::relabel(const Grid& grid, const Index& current, Labels& labels) const {
	assert(current.n_layer < grid.layerCount());

	const std::size_t current_v = index2val(current, grid.size());

	unsigned label = std::numeric_limits<unsigned>::max();

	// horizontal labels
	if(current.pos.x > 0)
		updateLabel(label, current, Index {V2i(current.pos.x-1, current.pos.y), current.n_layer}, grid, labels);

	if(current.pos.x < grid.size().x - 1)
		updateLabel(label, current, Index {V2i(current.pos.x+1, current.pos.y), current.n_layer}, grid, labels);

	if(current.pos.y > 0)
		updateLabel(label, current, Index {V2i(current.pos.x, current.pos.y-1), current.n_layer}, grid, labels);

	if(current.pos.y < grid.size().y - 1)
		updateLabel(label, current, Index {V2i(current.pos.x, current.pos.y+1), current.n_layer}, grid, labels);

	// vertical layer "down" - last layer leads to sink, which has always a label of 0
	if(current.n_layer+1 < grid.layerCount())
		updateLabel(label, current, Index{V2i(current.pos.x, current.pos.y), current.n_layer+1}, grid, labels);
	else if(grid.edge(current, Index{current.pos, current.n_layer+1}).residualCapacity() > 0)
		label = 0;

	// vertical layer "up" - first layer leads to source, which has always a label of N
	if(current.n_layer > 0)
		updateLabel(label, current, Index {V2i(current.pos.x, current.pos.y), current.n_layer-1}, grid, labels);
	else
		label = std::min(label, unsigned(grid.size().x * grid.size().y));

	// if an update is needed, do update
	if(label < std::numeric_limits<unsigned>::max()) {
		// two variants possible - < means update only UP (faster, but less accurate); != means any update (slower, more accurate?)
		assert(label+1 != labels[current_v]);
		labels[current_v] = label+1;

		return true;
	}

	return false;
}

std::size_t Graph::relabelAll(const Grid& grid, Labels& labels, ActiveQueue& active) {
	// back-tracing BFS
	labels.clear(grid.size().x * grid.size().y * grid.layerCount() + 1);

	std::size_t counter = 0;

	// first queue the bottom layer nodes, where the "forward" residual capacity is non-zero
	std::queue<Index> queue;
	for(int y=0; y<grid.size().y; ++y)
		for(int x=0; x<grid.size().x; ++x) {
			const Index current {V2i(x, y), unsigned(grid.layerCount()-1)};
			const Index target {V2i(x, y), unsigned(grid.layerCount())};

			if(grid.edge(current, target).residualCapacity() > 0) {
				queue.push(current);
				labels[index2val(current, grid.size())] = 1;

				++counter;
			}
		}

	// recurse
	while(!queue.empty()) {
		const Index current = queue.front();
		queue.pop();

		const unsigned current_val = labels[index2val(current, grid.size())];

		if(current.pos.x > 0) {
			const Index source {V2i(current.pos.x-1, current.pos.y), current.n_layer};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}

		if(current.pos.x < grid.size().x-1) {
			const Index source {V2i(current.pos.x+1, current.pos.y), current.n_layer};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}

		if(current.pos.y > 0) {
			const Index source {V2i(current.pos.x, current.pos.y-1), current.n_layer};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}

		if(current.pos.y < grid.size().y-1) {
			const Index source {V2i(current.pos.x, current.pos.y+1), current.n_layer};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}

		if(current.n_layer > 0) {
			const Index source {V2i(current.pos.x, current.pos.y), current.n_layer-1};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}

		if(current.n_layer < grid.layerCount()-1) {
			const Index source {V2i(current.pos.x, current.pos.y), current.n_layer+1};

			if(grid.edge(source, current).residualCapacity() > 0) {
				if(labels[index2val(source, grid.size())] > current_val + 1) {
					labels[index2val(source, grid.size())] = current_val + 1;
					queue.push(source);

					++counter;
				}
			}
		}
	}

	// for(std::size_t a=0; a<labels.size(); ) {
	// 	std::cout << labels[a] << " ";

	// 	++a;

	// 	if(a % m_size.x == 0)
	// 		std::cout << std::endl;
	// 	if(a % (m_size.x * m_size.y) == 0)
	// 		std::cout << std::endl;
	// }
	// std::cout << std::endl;

	active.relabel(labels);

	return counter;
}

void Graph::pushRelabelSolve(Grid& grid) {
	assert(grid.layerCount() > 1);

	Labels label(grid.size().x * grid.size().y * grid.layerCount(), grid.size().x * grid.size().y * grid.layerCount());
	ActiveQueue queue(grid.size().x * grid.size().y * grid.layerCount());

	// first of all, push from source - infinite capacity means maximum excess for each cell bordering with source
	for(int a=0; a<grid.size().x * grid.size().y; ++a)
		queue.push(ActiveQueue::Item(
			std::size_t(a),
			std::numeric_limits<int>::max() / 4,  // an arbitrary "large" number
			label[a]
		));

	// set the labels to the minimal distance to sink
	relabelAll(grid, label, queue);

	int gap_counter = 0;
	std::size_t relabel_counter = 0;
	std::size_t iterations = 0;

	int gap_threshold = label.size();

	while(!queue.empty() || iterations == 0) {
		++gap_counter;
		++relabel_counter;
		++iterations;

		// get an active cell
		ActiveQueue::Item current = queue.pop();
		const Index current_i = val2index(current.index, grid.size());

		// active cell - repeat pushing for as long as possible
		while(current.excess > 0) {

			// try to push all directions

			pushVertDown(grid, current_i, label, current.excess, queue);

			if(current.excess > 0 && current_i.pos.x > 0) {
				const Index next_i {V2i(current_i.pos.x-1, current_i.pos.y), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.x < grid.size().x-1) {
				const Index next_i {V2i(current_i.pos.x+1, current_i.pos.y), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.y > 0) {
				const Index next_i {V2i(current_i.pos.x, current_i.pos.y-1), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.y < grid.size().y-1) {
				const Index next_i {V2i(current_i.pos.x, current_i.pos.y+1), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0)
				pushVertUp(grid, current_i, label, current.excess, queue);

			// if there is any excess left, relabel
			if(current.excess > 0)
				relabel(grid, current_i, label);
		}

		// no matter what happens, at the end of the iterations above, the first layer should have 0 excess
		assert(current_i.n_layer > 0 || current.excess == 0);

		// push back to the end of the queue - not fully processed yet, couldn't push or relabel (only allowed up) and excess is not fully processed
		if(current.excess > 0)
			queue.push(current);

		// find the gap optimisation
		// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3 (1994): 90-96.
		if(gap_counter == gap_threshold) {
			gap_counter = 0;
			gap_threshold = queue.size();

			label.relabelGap();
		}

		// global relabelling optimisation
		// based on Cherkassky, Boris V. and Goldberg, Andrew V. "On implementing push-relabel method for the maximum flow problem.", Section 3
		if(relabel_counter == grid.size().x * grid.size().y * grid.layerCount()) {
			relabel_counter = 0;

			auto ctr = relabelAll(grid, label, queue);
			std::cout << "relabelled " << ctr << std::endl;
		}

		if(iterations % (grid.size().x * grid.size().y) == 0)
			std::cout << "active count = " << queue.size() << std::endl;
	}

	assert(queue.checkEmpty());
}

}
