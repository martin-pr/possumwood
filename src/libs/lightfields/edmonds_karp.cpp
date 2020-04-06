#include "edmonds_karp.h"

#include <queue>

#include <tbb/blocked_range2d.h>
#include <tbb/task_group.h>

#include "index.h"
#include "bfs_visitors.h"
#include "graph.h"

namespace lightfields {

namespace {

struct QueueItem {
	Index current, parent;
};

int flow(Graph& grid, const BFSVisitors& visitors, const Index& end) {
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

void doFlow(Graph& grid, const BFSVisitors& visitors, const Index& end, int flow) {
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

bool iterate(Graph& grid, const tbb::blocked_range2d<int>& range) {
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

}

void EdmondsKarp::solve(Graph& grid) {
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

}
