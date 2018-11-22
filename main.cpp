// CPP program to demonstrate multithreading 
// using three different callables. 
#include <iostream> 
#include <thread> 
#include <chrono>
#include <unistd.h>
#include <mqueue.h>
#include <string>
#include <string.h>
#include <fstream>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h> 
#include <list>
#include <vector>
using namespace std; 
//#define DEBUG_LOG_ENABLE

#define NODE_COUNT 6
#define DEBUG_ERROR_LOG_ENABLE
#define SERVER_QUEUE_NAME   "/nodeclient-1"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10
#define PACKET_SIZE 19
#define MAX_NEIGHBORS 20
int sendMessageToNode(int nodeNumber, int message,mqd_t qd_server);
list<int> getNeighbours(int nodeNumber);
class BaseStation
{
private:
	list<int> neighbours;
	string messageQueueRcv;
	char packet[PACKET_SIZE];
public:
	mqd_t messageQueueHandles[NODE_COUNT];
	int init();
	void startListening();
	void sendBroadcast();
};
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
};

bool Node::last_node(char *packet)
{
    int i;
    for(i=6;i<16;i++)
    {
        if(packet[i]==0x00)
            break;
    }
    if((packet[i+1]==0x00)&&(i%2==0))
    {
        return true;
    }
    return false;
}

bool Node::node_in_packet(char *packet, int *locat)
{
    int i;
    for(i=6;i<16;i+=2)
    {
        if(packet[i]==(id))
        {
            *locat=i;
            return true;
        }
    }
    return false;
}
void Node::packet_arrange_sync(char *packet)
{
    int i,j;
    char tmp[10];
    for(i=6;i<16;i++)
    {
        if(packet[i]==0x00)
            break;
    }
    packet[i]=id;
    packet[++i]=energy;
    for(j=6;j<=i;j+=2)
    {
        tmp[j-6]=packet[i-j+5];//node
        tmp[j-5]=0x00;//energy
    }
    for(j=6;j<=i;j++)
        packet[j]=tmp[j-6];
    packet[7]=energy;
    for(j=i+1;j<16;j++)
        packet[j]=0x56;
}
char* Node::find_neighbor(char *packet)
{
    int i,loc=0;
    if(node_in_packet(packet,&loc))
    {
        packet[loc+1]=energy;
        return packet;
    }
	if(last_node(packet))
    {
        *(packet+1)='N';
        *(packet+2)='E';
        *(packet+3)='A';
        *(packet+4)='C';
        *(packet+5)='K';
        packet_arrange_sync(packet);
        return packet;
    }
    return NULL;
//location1:;
}

char* Node::ack_neighbor(char *packet)
{
    int i,loc=0;
    if(node_in_packet(packet,&loc))
    {
        packet[loc+1]=energy;
	return packet;
    }
    return NULL;
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
		neighbours.push_back(3);
		neighbours.push_back(1);
	}
	else if(nodeNumber == 3)
	{
		neighbours.push_back(4);
		neighbours.push_back(2);
	}
	else if(nodeNumber == 4)
	{
		neighbours.push_back(5);
		neighbours.push_back(3);
	}
	else if(nodeNumber == 5)
	{
		neighbours.push_back(1);
		neighbours.push_back(4);
	}
	else
	{
	}
	return neighbours;
}

int Node::init()
{
	//mqd_t qd_server;
	/*struct mq_attr attr;
	neighbours = getNeighbours(id);
	list <int> :: iterator it; 
	attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0; 
	for(it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		messageQueueRcv = "/node";
		messageQueueRcv += to_string(*it);
		messageQueueRcv +="rcv";
		if ((messageQueueHandles[*it] = mq_open (messageQueueRcv.c_str(), O_RDWR | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) 
		{
			cerr<<"Error Opening message Queue :"<<id<<endl;
		}
		else
		{
			cout<<" \n N: Opened Message Queue"<<messageQueueRcv<<endl;
			//messageQueueHandles.push_back(qd_server);
		}		
	}
	#ifdef DEBUG_LOG_ENABLE
	cout<<" Node Init Complete\n";
	#endif*/
}
int BaseStation::init()
{
	/*struct mq_attr attr;
	mqd_t qd_server;
	neighbours = getNeighbours(9);
	list <int> :: iterator it;
	for(it = neighbours.begin(); it != neighbours.end(); ++it)
	{
		messageQueueRcv = "/node";	
		messageQueueRcv += to_string(*it);
		messageQueueRcv +="rcv";

    	attr.mq_flags = 0;
    	attr.mq_maxmsg = MAX_MESSAGES;
    	attr.mq_msgsize = MAX_MSG_SIZE;
    	attr.mq_curmsgs = 0;
		if ((qd_server = mq_open (messageQueueRcv.c_str(), O_RDWR | O_CREAT, QUEUE_PERMISSIONS, &attr)) == 1) 
		{
			cerr<<"Error Opening message Queue :"<<*it<<endl;
		}
		else
		{
			cout<<" \n BS: Opened Message Queue"<<messageQueueRcv<<endl;
			cout<<" Node:"<<messageQueueRcv<<" "<<qd_server;
			//messageQueueHandles.push_back(qd_server);
		}		
	}
	#ifdef DEBUG_LOG_ENABLE	
	cout<<" BaseStation Init Complete: No Of neighbours"<<neighbours.size()<<"\n";
	#endif*/
}

void Node::startListening(){
	char client_queue_name [64];
	char *packet;
    mqd_t qd_client,qd_forward;   // queue descriptors


    // create the client queue for receiving messages from server
    sprintf (client_queue_name, "/nodeclient-%d", id);

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    string logfile = "node";
	logfile += to_string(id);
	logfile +=".txt";
    ofstream myfile (logfile.c_str());
    myfile.close();

    if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
    }

	//cout <<"opened client queue  :"<<id<<endl;
    char in_buffer [MSG_BUFFER_SIZE];

    char temp_buf [10];

    while (1) {

        if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Client: mq_receive");
            exit (1);
        }
        // display token received from server
        myfile.open(logfile.c_str(),ios::app);
		cout <<"Received";
		for(int i=0;i<19;i++)
		{
			printf(" %x", in_buffer[i]);
		 	myfile << in_buffer[i];
		}
		myfile << "\n";
		myfile.close();
					
		cout<<endl;

		if((in_buffer[1]=='N')&&(in_buffer[2]=='E')&&(in_buffer[3]=='I')&&(in_buffer[4]=='G')&&(in_buffer[5]=='H'))
        {
			packet=find_neighbor(in_buffer);
			cout<<"Client ID: "<<id<<endl;

			if(packet!=NULL)
			{
				neighbours = getNeighbours(id-1);
				list <int> :: iterator it;
				
				for(it = neighbours.begin(); it != neighbours.end(); ++it)
				{
					messageQueueRcv = "/nodeclient-";
					messageQueueRcv+=to_string(*it);
					if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_WRONLY)) == -1) {
					perror ("Forward: mq_open (qdforward)");
					continue;
					}
					if (mq_send (qd_forward, packet, PACKET_SIZE, 0) == -1) {
						perror ("Forward: Not able to send message to client");
						continue;
					}
					cout << "opened message queue forward: " <<messageQueueRcv.c_str()<<endl;

					cout <<"forwarded";
					for(int i=0;i<19;i++)
						printf(" %x", packet[i]);
					
					cout<<endl;
					mq_close(qd_forward);
					std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				}
			// display token received from server
			//printf ("forward: Token received from server: %s\n\n", in_buffer);
			}
		}
        else if((in_buffer[1]=='N')&&(in_buffer[2]=='E')&&(in_buffer[3]=='A')&&(in_buffer[4]=='C')&&(in_buffer[5]=='K'))
        {
			packet=ack_neighbor(in_buffer);
			if(packet!=NULL)
			{
				neighbours = getNeighbours(id-1);
				list <int> :: iterator it;
				
				for(it = neighbours.begin(); it != neighbours.end(); ++it)
				{
					messageQueueRcv = "/nodeclient-";
					messageQueueRcv+=to_string(*it);
					if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_WRONLY)) == -1) {
					perror ("Forward: mq_open (qdforward)");
					continue;
					}
					cout << "opened message queue forward: " <<messageQueueRcv.c_str()<<endl;
					if (mq_send (qd_forward, packet, PACKET_SIZE, 0) == -1) {
						perror ("Forward: Not able to send message to client");
						continue;
					}
					cout<<"Client ID forward: "<<packet[3]<<endl;
					 std::this_thread::sleep_for(std::chrono::milliseconds(2000));
					mq_close(qd_forward);
				}
				// display token received from server
				//printf ("forward: Token received from server: %s\n\n", in_buffer);
			}
		}
    }


    if (mq_close (qd_client) == -1) {
        perror ("Client: mq_close");
        exit (1);
    }

    if (mq_unlink (client_queue_name) == -1) {
        perror ("Client: mq_unlink");
        exit (1);
    }
    printf ("Client: bye\n");
}
void BaseStation::sendBroadcast()
{
 	mqd_t qd_client[MAX_NEIGHBORS];   // queue descriptors
    long token_number = 1; // next token to be given to client

    printf ("Server: Hello, World!\n");

    struct mq_attr attr;

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
    char in_buffer [MSG_BUFFER_SIZE];

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));


        // send reply message to client

        packet[0]='#';
		packet[1]='N';
		packet[2]='E';
		packet[3]='I';
		packet[4]='G';
		packet[5]='H';
		packet[6]=0x01;
		packet[7]=0x24;
		packet[8]=0x02;
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
		for(int loop=0;loop<looper;loop++)
        if (mq_send (qd_client[loop], packet, PACKET_SIZE, 0) == -1) {
            perror ("Server: Not able to send message to client");
            continue;
        }

        //cout<<"Server: response sent to client.\n";
        token_number++;
    }

}


int main() 
{

	Node nodes[NODE_COUNT];
	BaseStation baseStation;
	thread nodeListening[NODE_COUNT];
	for(int i=0; i< NODE_COUNT; i++){
		nodes[i].setId(i+1,(i+1)*10);
	}

	baseStation.init();
	for(int i=0; i< NODE_COUNT; i++){
		nodes[i].init();
	}
	
	for(int i=0; i< NODE_COUNT; i++){
		nodeListening[i]= std::thread (&Node::startListening,&nodes[i]);
	
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//baseStation.sendBroadcast();
	baseStation.sendBroadcast();
	for(int i=0; i< NODE_COUNT; i++){
		nodeListening[i].join();
	}

	return 0; 
} 
	