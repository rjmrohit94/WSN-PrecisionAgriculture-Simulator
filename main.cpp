#include <iostream> 
#include <thread> 
#include <chrono>
#include <unistd.h>
#include <mqueue.h>
#include <string>
#include <string.h>
#include <fstream>
#include <fcntl.h>         
#include <sys/stat.h> 
#include <vector>
#include <bits/stdc++.h> 
#include <utility>
#include "BaseStation.h"
#include "Node.h"
#include "wsn_common.h"
using namespace std; 

typedef pair<int,int> iPair; 
struct timespec RcvCmdWaitTime;
const char nodeid[NUM_NODES]={0x01,0x02,0x03,0x04,0x05,0x06};
const int nodeegy[NUM_NODES]={0x24,0x34,0x64,0x21,0x45,0x52};
route_t route[NODE_COUNT];


int last_node_server(char *packet)
{
    int i;
    for(i=6;i<16;i++)
    {
        if(packet[i]==0x00)
            break;
    }
    if((packet[i+1]==0x56))
    {
        return packet[i-1];
    }
    return 0;
}

int idtoindex(int l, int r, int x) //binary search algorithm
{ 
   if (r >= l) 
   { 
        int mid = l + (r - l)/2; 
  
        if (nodeid[mid] == x)   
            return mid; 
  
        if (nodeid[mid] > x)  
            return idtoindex(l, mid-1, x); 
  
        return idtoindex(mid+1, r, x); 
   } 
  
   return -1; 
} 

bool notinandappend(char value, char* neigh)
{
	int i;
	for (i=0;i<100;i++)
	{
		if(value==*(neigh))
		{
			return false;
		}
		if (*neigh==0x00)
		{
			*neigh=value;
			break;
		}
		neigh++;
	}
	return true;
}

void sortascending(char* neigh)
{
	int i,j,n;
	char key;
	for(i=0;i<100;i++)
	{
		if (*(neigh++)==0x00)
			break;
	}
	n=i;
	neigh-=(n+1);
	for (i = 1; i < n; i++) //insertion sort
   	{ 
       key = *(neigh+i); 
       j = i-1; 
       while (j >= 0 && *(neigh+j) > key) 
       { 
           *(neigh+j+1) = *(neigh+j); 
           j = j-1; 
       } 
       *(neigh+j+1) = key; 
   } 
	
	
}

bool waitfornack(char* ptr)
{
	int flag=0,i,j;
	char packet[19];
	for(i=0;i<19;i++)
	   	packet[i]=*ptr++;
	ptr-=19;
	
	if((packet[PACKET_SIZE-1]=='P')&&(packet[PACKET_SIZE-2]=='O')&&(packet[PACKET_SIZE-3]=='E')) //whether we have completed reception
        {
            if((packet[1]=='N')&&(packet[2]=='E')&&(packet[3]=='A')&&(packet[4]=='C')&&(packet[5]=='K'))
            {
                if(packet[0]=='#')
                {
                    if(last_node_server(packet)==BASE_ID)
                	{
                		if((notinandappend(packet[6],route[idtoindex(0,NUM_NODES-1,packet[8])].neigh))==true)
                		{
                			sortascending(route[idtoindex(0,NUM_NODES-1,packet[8])].neigh);
                			return true;
                		}
                	}	
                }
            }
        }
    return false;
}


list<int> getNeighbours(int nodeNumber)
{
	list<int> neighbours;
	if(nodeNumber == 1)
	{
		neighbours.push_back(2);
		neighbours.push_back(5);
	}
	else if(nodeNumber == 2)
	{
		neighbours.push_back(1);
		neighbours.push_back(3);
	}
	else if(nodeNumber == 3)
	{
		neighbours.push_back(2);
		neighbours.push_back(4);
		neighbours.push_back(5);
		neighbours.push_back(6);
	}
	else if(nodeNumber == 4)
	{
		neighbours.push_back(3);
		neighbours.push_back(5);
	}
	else if(nodeNumber == 5)
	{
		neighbours.push_back(1);
		neighbours.push_back(3);
		neighbours.push_back(4);
		neighbours.push_back(6);
	}
	else if(nodeNumber == 6)
	{
		neighbours.push_back(5);
		neighbours.push_back(3);
	}
	else
	{
	}
	return neighbours;
}

int main() 
{

	Node nodes[NODE_COUNT];
	int Ver=NUM_NODES;
	BaseStation baseStation(Ver);
	thread nodeListening[NODE_COUNT];
	for(int i=0;i<NUM_NODES;i++)
    {
	   	for(int j=0;j<10;j++)
		{
			route[i].node[j]=0x00;
			route[i].node_energy[j]=0x00;
		}
		for(int j=0;j<100;j++)
		{
			route[i].neigh[j]=0x00;
		}
    }
	for(int i=0; i< NODE_COUNT; i++){
		nodes[i].setId(i+1,nodeegy[i]);
	}

	baseStation.init();
	for(int i=0; i< NODE_COUNT; i++){
		nodes[i].init();
	}
	
	for(int i=0; i< NODE_COUNT; i++){
		nodeListening[i]= std::thread (&Node::startListening,&nodes[i]);
	
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	baseStation.sendBroadcast();
	for(int i=0; i< NODE_COUNT; i++){
		nodeListening[i].join();
	}

	return 0; 
} 
	
