#ifndef BASE_ST_H
#define BASE_ST_H

#include <list>
#include <vector>
#include <string>
#include <mqueue.h>
#include <iostream>
#include <bits/stdc++.h> 
#include <utility>
#include "wsn_common.h"

using namespace std;

typedef int vertex_t;
typedef double weight_t;
class BaseStation 
{
private:
	int V; 
	list<int> neighbours;
	string messageQueueRcv;
	char packet[PACKET_SIZE];
	const weight_t max_weight = std::numeric_limits<double>::infinity();
 
	struct neighbor {
    vertex_t target;
    weight_t weight;
    neighbor(vertex_t arg_target, weight_t arg_weight)
        : target(arg_target), weight(arg_weight) { }
	};

	typedef std::vector<std::vector<neighbor> > adjacency_list_t;

public:
	mqd_t messageQueueHandles[NODE_COUNT];
	int init();
	int last_node_server(char *packet);
	
	void startListening();
	void sendBroadcast();
	void DijkstraComputePaths(vertex_t source,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous);
	list<vertex_t> DijkstraGetShortestPathTo(vertex_t vertex, const std::vector<vertex_t> &previous);
	BaseStation(int Ver);
};

#endif