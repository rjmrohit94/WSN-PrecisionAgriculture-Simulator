/******************************************************************************************
* File Name:
* Author(s):
* Desciption:
* Created Date:
* License:
* 
*********************************************************************************************/

#ifndef WSN_COMMON_H
#define WSN_COMMON_H

#include <list>
using namespace std;
#define NODE_COUNT 6
#define DEBUG_ERROR_LOG_ENABLE
#define SERVER_QUEUE_NAME   "/nodeclient-1"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10
#define PACKET_SIZE 19
#define MAX_NEIGHBORS 20
#define BASE_ID 0x01
#define NUM_NODES NODE_COUNT
#define INF 0x3f3f3f3f 

typedef struct{
	char node[10];
	char node_energy[10];
	char neigh[100];
}route_t;

int last_node_server(char *packet);
int idtoindex(int l, int r, int x);
bool notinandappend(char value, char* neigh);
void sortascending(char* neigh);
bool waitfornack(char* ptr);
list<int> getNeighbours(int nodeNumber);
int sendMessageToNode(int nodeNumber, int message,mqd_t qd_server);
#endif