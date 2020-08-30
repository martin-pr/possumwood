#include "push_relabel.h"

#include <tbb/task_group.h>

#include <cassert>
#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include <thread>

#include "active_queue.h"
#include "graph.h"
#include "index.h"
#include "labels.h"

namespace lightfields {

namespace {

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

bool pushHoriz(Graph& grid, const Index& current, const Index& next, const Labels& label, int& excess,
               ActiveQueue& queue) {
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

bool pushVertDown(Graph& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue) {
	assert(current.n_layer < grid.layerCount());

	const Index next{current.pos, current.n_layer + 1};
	const std::size_t current_v = index2val(current, grid.size());
	const std::size_t next_v = index2val(next, grid.size());

	assert(current_v < label.size());

	if((label[current_v] == 1 && current.n_layer + 1 == grid.layerCount()) ||
	   (next_v < label.size() && label[current_v] == label[next_v] + 1)) {
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

bool pushVertUp(Graph& grid, const Index& current, const Labels& label, int& excess, ActiveQueue& queue) {
	assert(current.n_layer < grid.layerCount());

	const std::size_t current_v = index2val(current, grid.size());

	// flowing "back to the source" - no explicit edge, just flow everything back
	if(current.n_layer == 0 && label[current_v] == unsigned(grid.size().x * grid.size().y + 1)) {
		excess = 0;
		return true;
	}

	// flow between layers with explicit edges
	if(current.n_layer > 0) {
		const Index next{current.pos, current.n_layer - 1};
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

void updateLabel(unsigned& label, const Index& current, const Index& target, const Graph& grid, const Labels& labels) {
	auto& e = grid.edge(current, target);
	if(e.residualCapacity() > 0) {
		const std::size_t target_v = index2val(target, grid.size());
		label = std::min(label, unsigned(labels[target_v]));
	}
}

bool relabel(const Graph& grid, const Index& current, Labels& labels) {
	assert(current.n_layer < grid.layerCount());

	const std::size_t current_v = index2val(current, grid.size());

	unsigned label = std::numeric_limits<unsigned>::max();

	// horizontal labels
	if(current.pos.x > 0)
		updateLabel(label, current, Index{V2i(current.pos.x - 1, current.pos.y), current.n_layer}, grid, labels);

	if(current.pos.x < grid.size().x - 1)
		updateLabel(label, current, Index{V2i(current.pos.x + 1, current.pos.y), current.n_layer}, grid, labels);

	if(current.pos.y > 0)
		updateLabel(label, current, Index{V2i(current.pos.x, current.pos.y - 1), current.n_layer}, grid, labels);

	if(current.pos.y < grid.size().y - 1)
		updateLabel(label, current, Index{V2i(current.pos.x, current.pos.y + 1), current.n_layer}, grid, labels);

	// vertical layer "down" - last layer leads to sink, which has always a label of 0
	if(current.n_layer + 1 < grid.layerCount())
		updateLabel(label, current, Index{V2i(current.pos.x, current.pos.y), current.n_layer + 1}, grid, labels);
	else if(grid.edge(current, Index{current.pos, current.n_layer + 1}).residualCapacity() > 0)
		label = 0;

	// vertical layer "up" - first layer leads to source, which has always a label of N
	if(current.n_layer > 0)
		updateLabel(label, current, Index{V2i(current.pos.x, current.pos.y), current.n_layer - 1}, grid, labels);
	else
		label = std::min(label, unsigned(grid.size().x * grid.size().y));

	// if an update is needed, do update
	if(label < std::numeric_limits<unsigned>::max()) {
		// two variants possible - < means update only UP (faster, but less accurate); != means any update (slower, more
		// accurate?)
		assert(label + 1 != labels[current_v]);
		labels[current_v] = label + 1;

		return true;
	}

	return false;
}

bool relabelBFS(const Index& source, const Index& current, const unsigned current_val, Labels& labels,
                std::queue<Index>& queue, const Graph& grid) {
	if(grid.edge(source, current).residualCapacity() > 0) {
		if(labels[index2val(source, grid.size())] > current_val + 1) {
			labels[index2val(source, grid.size())] = current_val + 1;
			queue.push(source);

			return true;
		}
	}

	return false;
}

std::size_t relabelAll(const Graph& grid, Labels& labels, ActiveQueue& active) {
	// back-tracing BFS
	labels.clear(grid.size().x * grid.size().y * grid.layerCount() + 1);

	std::size_t counter = 0;

	// first queue the bottom layer nodes, where the "forward" residual capacity is non-zero
	std::queue<Index> queue;
	for(int y = 0; y < grid.size().y; ++y)
		for(int x = 0; x < grid.size().x; ++x) {
			const Index current{V2i(x, y), unsigned(grid.layerCount() - 1)};
			const Index target{V2i(x, y), unsigned(grid.layerCount())};

			if(grid.edge(current, target).residualCapacity() > 0) {
				queue.push(current);
				labels[index2val(current, grid.size())] = 1;

				++counter;
			}
		}

	// back BFS parsing
	while(!queue.empty()) {
		const Index current = queue.front();
		queue.pop();

		const unsigned current_val = labels[index2val(current, grid.size())];

		if(current.pos.x > 0)
			counter += relabelBFS(Index{V2i(current.pos.x - 1, current.pos.y), current.n_layer}, current, current_val,
			                      labels, queue, grid);

		if(current.pos.x < grid.size().x - 1)
			counter += relabelBFS(Index{V2i(current.pos.x + 1, current.pos.y), current.n_layer}, current, current_val,
			                      labels, queue, grid);

		if(current.pos.y > 0)
			counter += relabelBFS(Index{V2i(current.pos.x, current.pos.y - 1), current.n_layer}, current, current_val,
			                      labels, queue, grid);

		if(current.pos.y < grid.size().y - 1)
			counter += relabelBFS(Index{V2i(current.pos.x, current.pos.y + 1), current.n_layer}, current, current_val,
			                      labels, queue, grid);

		if(current.n_layer > 0)
			counter += relabelBFS(Index{V2i(current.pos.x, current.pos.y), current.n_layer - 1}, current, current_val,
			                      labels, queue, grid);

		if(current.n_layer < grid.layerCount() - 1)
			counter += relabelBFS(Index{V2i(current.pos.x, current.pos.y), current.n_layer + 1}, current, current_val,
			                      labels, queue, grid);
	}

	active.relabel(labels);

	return counter;
}

}  // namespace

void PushRelabel::solve(Graph& grid) {
	assert(grid.layerCount() > 1);

	Labels label(grid.size().x * grid.size().y * grid.layerCount(), grid.size().x * grid.size().y * grid.layerCount());
	ActiveQueue queue(grid.size().x * grid.size().y * grid.layerCount());

	// first of all, push from source - infinite capacity means maximum excess for each cell bordering with source
	for(int a = 0; a < grid.size().x * grid.size().y; ++a)
		queue.push(ActiveQueue::Item(std::size_t(a),
		                             std::numeric_limits<int>::max() / 4,  // an arbitrary "large" number
		                             label[a]));

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
				const Index next_i{V2i(current_i.pos.x - 1, current_i.pos.y), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.x < grid.size().x - 1) {
				const Index next_i{V2i(current_i.pos.x + 1, current_i.pos.y), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.y > 0) {
				const Index next_i{V2i(current_i.pos.x, current_i.pos.y - 1), current_i.n_layer};
				pushHoriz(grid, current_i, next_i, label, current.excess, queue);
			}

			if(current.excess > 0 && current_i.pos.y < grid.size().y - 1) {
				const Index next_i{V2i(current_i.pos.x, current_i.pos.y + 1), current_i.n_layer};
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

		// push back to the end of the queue - not fully processed yet, couldn't push or relabel (only allowed up) and
		// excess is not fully processed
		if(current.excess > 0)
			queue.push(current);

		// find the gap optimisation
		// based on Cherkassky, Boris V. "A fast algorithm for computing maximum flow in a network." Collected Papers 3
		// (1994): 90-96.
		if(gap_counter == gap_threshold) {
			gap_counter = 0;
			gap_threshold = queue.size();

			label.relabelGap();
		}

		// global relabelling optimisation
		// based on Cherkassky, Boris V. and Goldberg, Andrew V. "On implementing push-relabel method for the maximum
		// flow problem.", Section 3
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

}  // namespace lightfields
