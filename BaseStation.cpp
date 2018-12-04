/******************************************************************************************
*
*
*
*
*
*
*
*
*********************************************************************************************/

#include "BaseStation.h"

extern route_t route[NODE_COUNT];
extern struct timespec RcvCmdWaitTime;

/******************************************************************************************
* Function Name :Constructor
* Description :
* Parameters :int
* Return Value: 
*********************************************************************************************/
BaseStation::BaseStation(int V) 
{ 
    this->V = V; 
    
} 

/******************************************************************************************
* Function Name :DijkstraComputePaths
* Description :
* Parameters :
* Return Value: 
*********************************************************************************************/
void BaseStation::DijkstraComputePaths(vertex_t source,
                          const adjacency_list_t &adjacency_list,
                          std::vector<weight_t> &min_distance,
                          std::vector<vertex_t> &previous)
{
    int n = adjacency_list.size();
    min_distance.clear();
    min_distance.resize(n, max_weight);
    min_distance[source] = 0;
    previous.clear();
    previous.resize(n, -1);
    std::set<std::pair<weight_t, vertex_t> > vertex_queue;
    vertex_queue.insert(std::make_pair(min_distance[source], source));
 
    while (!vertex_queue.empty()) 
    {
        weight_t dist = vertex_queue.begin()->first;
        vertex_t u = vertex_queue.begin()->second;
        vertex_queue.erase(vertex_queue.begin());
 
        // Visit each edge exiting u
		const std::vector<neighbor> &neighbors = adjacency_list[u];
        for (std::vector<neighbor>::const_iterator neighbor_iter = neighbors.begin();
             neighbor_iter != neighbors.end();
             neighbor_iter++)
        {
            vertex_t v = neighbor_iter->target;
            weight_t weight = neighbor_iter->weight;
            weight_t distance_through_u = dist + weight;
	    if (distance_through_u < min_distance[v]) {
	        vertex_queue.erase(std::make_pair(min_distance[v], v));
 
	        min_distance[v] = distance_through_u;
	        previous[v] = u;
	        vertex_queue.insert(std::make_pair(min_distance[v], v));
 
	    }
 
        }
    }
} 

/******************************************************************************************
* Function Name :DijkstraGetShortestPathTo
* Description :
* Parameters :
* Return Value: 
*********************************************************************************************/
std::list<vertex_t> BaseStation::DijkstraGetShortestPathTo(
    vertex_t vertex, const std::vector<vertex_t> &previous)
{
    std::list<vertex_t> path;
    for ( ; vertex != -1; vertex = previous[vertex])
        path.push_front(vertex);
    return path;
}

/******************************************************************************************
* Function Name :sendBroadcast
* Description :
* Parameters :
* Return Value: 
*********************************************************************************************/
void BaseStation::sendBroadcast()
{
 	mqd_t qd_client[MAX_NEIGHBORS],qd_cliafloop;   // queue descriptors
	vector <int> iternodelist;
	vector <int> calcnodelist;
	vector <int> difflist;
	vector<int>::iterator ip;
	std::vector<weight_t> min_distance;
    std::vector<vertex_t> previous;
	adjacency_list_t adjacency_list(V+1);
	//int whilecount=0;
    printf ("Server: Hello, World!\n");

    struct mq_attr attr;
	RcvCmdWaitTime.tv_sec = 0;
    RcvCmdWaitTime.tv_nsec = 250000;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    neighbours = getNeighbours(1);
	list <int> :: iterator it;
	int looper=0;
	for(it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		messageQueueRcv = "/nodeclient-";
		messageQueueRcv+=to_string(*it);
		if ((qd_client[looper] = mq_open (messageQueueRcv.c_str(), O_WRONLY)) == 1) {
				perror ("Server: Not able to open client queue");
		}
		looper++;
	}
	int fstnodeneighno=looper;
    char in_buffer [MSG_BUFFER_SIZE];
	packet[0]='#';
	packet[1]='N';
	packet[2]='E';
	packet[3]='I';
	packet[4]='G';
	packet[5]='H';
	packet[6]=0x01;
	packet[7]=0x24;
	packet[8]=0x00;
	packet[9]=0x00;
	packet[10]=0x00;
	packet[11]=0x00;
	packet[12]=0x00;
	packet[13]=0x00;
	packet[14]=0x00;
	packet[15]=0x00;
	packet[16]='E';
	packet[17]='O';
	packet[18]='P';
	for(int loop=0;loop<fstnodeneighno;loop++)
	if (mq_send (qd_client[loop], packet, PACKET_SIZE, 0) == -1) {
		perror ("Server: Not able to send message to client");
		continue;
	}
	
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	iternodelist.push_back(1);
	set_difference(iternodelist.begin(), iternodelist.end(), calcnodelist.begin(), calcnodelist.end(),inserter(difflist, difflist.begin()));
    while (1) {
		//cout<<"in while :"<<whilecount++<<endl;
		while(!difflist.empty())
		{
			for (auto ite : difflist)
			{	
				for(int j=0;route[ite-1].neigh[j]!=0;j++)
				{
					adjacency_list[ite].push_back(neighbor(route[ite-1].neigh[j],1));
					if (find(iternodelist.begin(), iternodelist.end(), route[ite-1].neigh[j]) == iternodelist.end()) {
						iternodelist.push_back(route[ite-1].neigh[j]);
					}
				}
				for (int i=1;i<NUM_NODES;i++)
				{
					for(int j=0;route[i].neigh[j]!=0;j++)
					{
						if((notinandappend(i+1,route[idtoindex(0,NUM_NODES-1,route[i].neigh[j])].neigh))==true)
							{
								sortascending(route[idtoindex(0,NUM_NODES-1,route[i].neigh[j])].neigh);
							}
					}
				}
				//cout<<"Element in diff list :"<<ite<<endl;
				sort(iternodelist.begin(), iternodelist.end());
				ip = unique(iternodelist.begin(), iternodelist.end()); 
				iternodelist.resize(distance(iternodelist.begin(), ip));
				/*for(auto irt=iternodelist.begin();irt!=iternodelist.end();irt++)
				{
					cout << "Iterbnid :" <<*irt<<endl;
				}*/
				DijkstraComputePaths(1, adjacency_list, min_distance, previous);
				list<vertex_t> path = DijkstraGetShortestPathTo(ite, previous);
				cout << "Path : ";
				looper=0;
				for (std::list<int>::const_iterator iterator = path.begin(), end = path.end(); iterator != end; ++iterator) {
					route[ite-1].node[looper]=*iterator;
					route[ite-1].node_energy[looper]=0;/*nodeegy[*iterator];*/
					std::cout << *iterator<<" ";
					looper++;
				}

				//copy(path.begin(), path.end(), std::ostream_iterator<vertex_t>(std::cout, " "));
				cout << std::endl;
				packet[0]='#';
				packet[1]='N';
				packet[2]='E';
				packet[3]='I';
				packet[4]='G';
				packet[5]='H';
				packet[6]=route[ite-1].node[0];
				packet[7]=0x24;//route[ite-1].node_energy[0];
				packet[8]=route[ite-1].node[1];
				packet[9]=route[ite-1].node_energy[1];
				packet[10]=route[ite-1].node[2];
				packet[11]=route[ite-1].node_energy[2];
				packet[12]=route[ite-1].node[3];
				packet[13]=route[ite-1].node_energy[3];
				packet[14]=route[ite-1].node[4];
				packet[15]=route[ite-1].node_energy[4];
				packet[16]='E';
				packet[17]='O';
				packet[18]='P';
				cout<<"Hello George"<<endl;
				for(int j=0;j<19;j++)
				{
					printf("%x  ",packet[j] );
				}
				cout<<endl;
				for(int loop=0;loop<fstnodeneighno;loop++)
				if (mq_timedsend (qd_client[loop], packet, PACKET_SIZE, 0,&RcvCmdWaitTime) == -1) {
					perror ("Server: Not able to send message to client");
					continue;
				}
				
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
				for(int i=0;i<NUM_NODES;i++)
				{
					for(int j=0;j<5;j++)
						{
							printf("%x  ",route[i].neigh[j] );
						}
						cout<<endl;
				}
				for(int j=0;route[ite-1].neigh[j]!=0;j++)
				{
					adjacency_list[ite].push_back(neighbor(route[ite-1].neigh[j],1));
					if (find(iternodelist.begin(), iternodelist.end(), route[ite-1].neigh[j]) == iternodelist.end()) {
						iternodelist.push_back(route[ite-1].neigh[j]);
					}
				}
				//cout<<"Element in diff list :"<<ite<<endl;
				sort(iternodelist.begin(), iternodelist.end());
				ip = unique(iternodelist.begin(), iternodelist.end()); 
				iternodelist.resize(distance(iternodelist.begin(), ip));
				/*for(auto irt=iternodelist.begin();irt!=iternodelist.end();irt++)
				{
					cout << "Iterbnid :" <<*irt<<endl;
				}*/
				calcnodelist.push_back(ite);
				sort(calcnodelist.begin(), calcnodelist.end());
				ip = unique(calcnodelist.begin(), calcnodelist.end()); 
				calcnodelist.resize(distance(calcnodelist.begin(), ip));
				for(auto irt=calcnodelist.begin();irt!=calcnodelist.end();irt++)
				{
					cout << "Calcnodelist :" <<*irt<<endl;
				}
			}
			while(!difflist.empty())
			{
				difflist.pop_back();
			}
			set_difference(iternodelist.begin(), iternodelist.end(), calcnodelist.begin(), calcnodelist.end(),inserter(difflist, difflist.begin()));
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		/*char temp='&';
		while(difflist.empty()&& (temp=='&'))
		{
			temp='*';
			for(int i=1;i<=NUM_NODES;i++)
			{
				messageQueueRcv = "/nodeclient-";
				messageQueueRcv+=to_string(i);
				if ((qd_cliafloop = mq_open (messageQueueRcv.c_str(), O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
					perror ("after while: mq_open (qdclient)");
				}
				if (mq_timedreceive (qd_cliafloop, in_buffer, MSG_BUFFER_SIZE, 0,&RcvCmdWaitTime) == -1) {
					perror ("after while: Not able to receive message from client");
					cout<<messageQueueRcv;
					cout<<" "<<i<<endl;
				}
				else{
					temp='&';
				}
				mq_close(qd_cliafloop);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		*/
	}
}


/******************************************************************************************
* Function Name :init
* Description :
* Parameters :
* Return Value: 
*********************************************************************************************/
int BaseStation::init()
{
	
}
