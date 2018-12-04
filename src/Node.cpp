/******************************************************************************************
* File Name:
* Author(s):
* Desciption:
* Created Date:
* License:
* 
*********************************************************************************************/

#include "Node.h"

extern struct timespec RcvCmdWaitTime;

/******************************************************************************************
* Function Name :last_node
* Description :
* Parameters :char *
* Return Value: bool 
*********************************************************************************************/
bool Node::last_node(char *packet)
{
    int i;
    for(i = 6; i < 16; i++)
    {
        if(packet[i] == 0x00)
            break;
    }
    if((packet[i+1] == 0x00) && ((i%2) == 0))
    {
        return true;
    }
    return false;
}

/******************************************************************************************
* Function Name :node_in_packet
* Description :
* Parameters :char *, int *
* Return Value: bool 
*********************************************************************************************/
bool Node::node_in_packet(char *packet, int *locat)
{
    int i;
    for(i = 6;i < 16; i += 2)
    {
        if(packet[i] == id)
        {
            *locat = i;
            return true;
        }
    }
    return false;
}

/******************************************************************************************
* Function Name :packet_arrange_sync
* Description :
* Parameters :char *
* Return Value: void 
*********************************************************************************************/
void Node::packet_arrange_sync(char *packet)
{
    int i,j;
    char tmp[10];
    for(i = 6; i < 16; i++)
    {
        if(packet[i] == 0x00)
            break;
    }
    packet[i] = id;
    packet[++i] = energy;
    for(j = 6; j <= i; j+=2)
    {
        tmp[j-6] = packet[i-j+5];//node
        tmp[j-5] = 0x00;//energy
    }
    for(j = 6; j <= i; j++)
    {    
        packet[j] = tmp[j-6];
    }
    packet[7] = energy;
    for(j=i+1;j<16;j++)
    {
        packet[j]=0x56;
    }
}

/******************************************************************************************
* Function Name :find_neighbor
* Description :
* Parameters :char *
* Return Value: char *
*********************************************************************************************/
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
/******************************************************************************************
* Function Name :ack_neighbor
* Description :
* Parameters :char *
* Return Value: char *
*********************************************************************************************/
char* Node::ack_neighbor(char *packet)
{
    int i,loc=0;
    if(node_in_packet(packet,&loc))
    {
		if(packet[loc+1]==0x00)
		{
			packet[loc+1]=energy;
			return packet;
		}
    }
    return NULL;
}
/******************************************************************************************
* Function Name :init
* Description :
* Parameters :void
* Return Value: int
*********************************************************************************************/
int Node::init()
{
    /* TBD*/
    return 0;
}

/******************************************************************************************
* Function Name :startListening
* Description :
* Parameters :void
* Return Value: void
*********************************************************************************************/
void Node::startListening()
{
    char client_queue_name [64];
    char *packet;
    mqd_t qd_client,qd_forward;   
    sprintf (client_queue_name, "/nodeclient-%d", id);

    struct mq_attr attr;
    RcvCmdWaitTime.tv_sec = 0;
    RcvCmdWaitTime.tv_nsec = 250000;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    string logfile = "node";
    logfile += to_string(id);
    logfile +=".txt";
    ofstream myfile (logfile.c_str());
    myfile.close();
    char in_buffer [MSG_BUFFER_SIZE];

    char temp_buf [10];

    while (1) 
    {
        if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) 
        {
            perror ("Client: mq_open (client)");
            exit (1);
        }
        if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) 
        {
            perror ("Client: mq_receive");
            exit (1);
        }
        myfile.open(logfile.c_str(),ios::app);
        //cout <<"Received : " <<id<<":";
        for(int i=0;i<19;i++)
        {
            //printf(" %x", in_buffer[i]);
            myfile << in_buffer[i];
        }
        myfile << "\n";
        myfile.close();
                    
        //cout<<endl;
        if(id!=1)
        {
            if((in_buffer[1]=='N')&&(in_buffer[2]=='E')&&(in_buffer[3]=='I')&&(in_buffer[4]=='G')&&(in_buffer[5]=='H'))
            {
                packet=find_neighbor(in_buffer);
                if(packet!=NULL)
                {
                    neighbours = getNeighbours(id);
                    list <int> :: iterator it;
                    
                    for(it = neighbours.begin(); it != neighbours.end(); ++it)
                    {
                        messageQueueRcv = "/nodeclient-";
                        messageQueueRcv+=to_string(*it);
                        if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_WRONLY)) == -1) 
                        {
                         perror ("Forward: mq_open (qdforward)");
                         continue;
                        }
                        if (mq_timedsend (qd_forward, packet, PACKET_SIZE, 0, &RcvCmdWaitTime) == -1) 
                        {
                            perror ("Forward: Not able to send message to client");
                            mq_close(qd_forward);
                            char temp='&';
                            while(temp=='&')
                            {
                                temp='*';
                                if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) 
                                {
                                    perror ("Forward: mq_open (qdforward)");
                                    continue;
                                }
                                if (mq_timedreceive (qd_forward, in_buffer, MSG_BUFFER_SIZE, 0,&RcvCmdWaitTime) == -1) 
                                {
                                    perror ("forward: Not able to receive message from client");
                                }
                                else
                                {
                                    temp='&';
                                }
                                mq_close(qd_forward);
                            }
                        }
                        mq_close(qd_forward);
                    
                    }
                }
            }
            else if((in_buffer[1]=='N')&&(in_buffer[2]=='E')&&(in_buffer[3]=='A')&&(in_buffer[4]=='C')&&(in_buffer[5]=='K'))
            {
                packet=ack_neighbor(in_buffer);
                if(packet!=NULL)
                {
                    neighbours = getNeighbours(id);
                    list <int> :: iterator it;
                    
                    for(it = neighbours.begin(); it != neighbours.end(); ++it)
                    {
                        messageQueueRcv = "/nodeclient-";
                        messageQueueRcv+=to_string(*it);
                        if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_WRONLY)) == -1) 
                        {
                            perror ("Forward: mq_open (qdforward)");
                            continue;
                        }
                        //cout << "opened message queue forward: " <<messageQueueRcv.c_str()<<endl;
                        if (mq_timedsend (qd_forward, packet, PACKET_SIZE, 0,&RcvCmdWaitTime) == -1) 
                        {
                            perror ("Forward: Not able to send message to client");
                            mq_close(qd_forward);
                            char temp='&';
                            while(temp=='&')
                            {
                                temp='*';
                                if ((qd_forward = mq_open (messageQueueRcv.c_str(), O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) 
                                {
                                    perror ("Forward: mq_open (qdforward)");
                                    continue;
                                }
                                if (mq_timedreceive (qd_forward, in_buffer, MSG_BUFFER_SIZE, 0,&RcvCmdWaitTime) == -1) 
                                {
                                    perror ("forward: Not able to receive message from client");
                                }
                                else
                                {
                                    temp='&';
                                }
                                mq_close(qd_forward);
                            }
                        }
                        mq_close(qd_forward);
                    }
                }
            }
            else{}
        }
        else
        {
            if(waitfornack(in_buffer))
            {
                //printf("Here\n");
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (mq_close (qd_client) == -1) 
        {
            perror ("Client: mq_close");
            exit (1);
        }
    }

    if (mq_unlink (client_queue_name) == -1) 
    {
        perror ("Client: mq_unlink");
        exit (1);
    }
    printf ("Client: bye\n");
}
