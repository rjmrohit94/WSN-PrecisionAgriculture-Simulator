/******************************************************************************************
* File Name:
* Author(s):
* Desciption:
* Created Date:
* License:
* 
*********************************************************************************************/

#ifndef NODE_H
#define NODE_H
#include <list>
#include <vector>
#include <string>
#include <mqueue.h>
#include <iostream>
#include <bits/stdc++.h> 
#include <utility>
#include "wsn_common.h"

using namespace std;

class Node
{
private:
	list<int> neighbours;
	int id;
	int energy;
	string messageQueueRcv;
	char message[60];
	char packet[PACKET_SIZE];
public:
		void setId(int id, int energy){
			this->id= id;
			this->energy=energy;
			messageQueueRcv = "/nodeclient-";
			messageQueueRcv += to_string(id);
			
		}
		int init();
		void startListening();
		bool last_node(char *packet);
		bool node_in_packet(char *packet, int *locat);
		void packet_arrange_sync(char *packet);
		char* find_neighbor(char *packet);
		char* ack_neighbor(char *packet);
		char* data_pack(char *packet);
		void packet_arrange_data(char *packet);
		bool last_nodedata(char *packet);
		bool node_in_packetdata(char *packet, int *locat);





};

#endif