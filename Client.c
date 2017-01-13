#include <stdio.h>
#include <stdlib.h>
#include <errno.h>		// defines perror(), herror()
#include <fcntl.h>		// set socket to non-blocking with fcntrl()
#include <unistd.h>
#include <string.h>
#include <assert.h>		//add diagnostics to a program
#include <netinet/in.h>		//defines in_addr and sockaddr_in structures
#include <arpa/inet.h>		//external definitions for inet functions
#include <netdb.h>		//getaddrinfo() & gethostbyname()
#include <sys/socket.h>		//
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/select.h>		// for select() system call only
#include <sys/time.h>		// time() & clock_gettime()


#define JOIN 2
#define SEND 4
#define FWD 3
#define ACK 7
#define NAK 5
#define ONLINE 8
#define OFFLINE 6
#define IDLE 9
#define STDIN fileno(stdin)

/* message header format */
struct msg_header
{
    unsigned int vrsn:9;
    unsigned int type:7;
    unsigned int length:16;
};

/* message attribute formate */
struct msg_attribute
{
    unsigned int type:16;
    unsigned int length:16;
    char payload[512];
};

/* message structure */
struct message
{
    struct msg_header header;
    struct msg_attribute attribute;
};

void error(char *msg)
{
    perror(msg);
    exit(0);
};

char msg_conv[1000];
 
 char *MsgCharConv(struct message Msg_s)
{
//insert for loop to clear msg_conv
 sprintf(msg_conv, "%d:%d:%d:%d:%d:%s", Msg_s.header.vrsn, Msg_s.header.type, Msg_s.header.length, Msg_s.attribute.type, Msg_s.attribute.length, Msg_s.attribute.payload); 
return (char *)msg_conv ;
}

/* initiate a join message to the server when connetting the server */
void joinServer(char *buf,int sockfd, int size_buf)
{
   struct  message msg;
    msg.header.vrsn=3; //set the protocal version is 3
    msg.header.type=JOIN; // join message is type 2
    msg.attribute.type=2; //username used in chatting
    msg.attribute.length=(4+size_buf);
    bzero((char*)&msg.attribute.payload,sizeof(msg.attribute.payload));
    int status=0;
    for (status=0;status<size_buf;status++)
    msg.attribute.payload[status]=buf[status];
    msg.header.length=(8+size_buf); 
	MsgCharConv(msg);
    if (send(sockfd,msg_conv,sizeof(msg_conv),0)<0)
    {error("Error writing to socket\n");}
    printf("The join message has been sent successfully\n");
}

/* send a message to the server */
void sendMessage(int sockfd)
{
    struct message msg;
    int size_buf;
    char buf[512];
    fgets(buf,sizeof(buf)-1,stdin);
    size_buf=strlen(buf)-1;
    if (buf[size_buf]=='\n')
    	{    
	buf[size_buf]='\0';
    	}
    msg.header.vrsn=3;
    msg.header.type=SEND; //send message is type 4
    msg.attribute.type=4; //message is type 4
    msg.attribute.length=(4+size_buf);
    bzero((char*)&msg.attribute.payload,sizeof(msg.attribute.payload));
    int status=0;
    for (status=0;status<size_buf;status++)
	{
    	msg.attribute.payload[status]=buf[status];
	}
    	msg.header.length=(8+size_buf);
    MsgCharConv(msg);
    if (send(sockfd,msg_conv,sizeof(msg_conv),0)<0)
    {error("Error writing to socket\n");}
}
/* handle server's message */
int rcv_msg(char *buff){
		struct message msg_r;
		int msg_header_vrsn; 
		int msg_header_type;
		int msg_header_Msg_len;
		int msg_attribute_type;
		int msg_attribute_len;
		char msg_attribute_payload[511];
		printf("\n"); 
       sscanf(buff,"%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%s", &msg_header_vrsn, &msg_header_type,&msg_header_Msg_len,&msg_attribute_type,&msg_attribute_len,msg_attribute_payload);	
		msg_r.header.vrsn=msg_header_vrsn-'0';
		msg_r.header.type=msg_header_type-'0';
		msg_r.header.length=msg_header_Msg_len-'0';
		msg_r.attribute.type=msg_attribute_type-'0';
		msg_r.attribute.length=msg_attribute_len-'0';
		printf("%s \n",msg_attribute_payload); 
		strncpy(msg_r.attribute.payload,msg_attribute_payload,511);
		//if(msg_r.header.vrsn==3 && msg_r.header.type==2 && msg_r.attribute.type==2)
		//{		
		//printf("%s", msg_r.attribute.payload); 
		//printf("\n Message Received \n");
		//}
               if (msg_r.header.type==ACK)
               {
                   printf("ACK message from the sender is:%s",msg_r.attribute.payload);
                }
               if (msg_r.header.type==NAK)
               {
                    printf("NAK message from the sender is:%s",msg_r.attribute.payload);
               }
              if (msg_r.header.type==ONLINE)
               {
              printf("ONLINE message from the sender is:%s",msg_r.attribute.payload);
               }
              if (msg_r.header.type==OFFLINE)
              {
              printf("OFFLINE message from the sender is:%s",msg_r.attribute.payload);
               }
             if (msg_r.header.type==IDLE)
             {
                 printf("%s is idle\n",msg_r.attribute.payload);
              }
	return(0);
}

    
int main(int argc,char * argv[])
{
    int sockfd,portno,connfd;
    struct sockaddr_in serv_addr;
    fd_set readfd;
    int fdmax;
    char username[16];
    int buf_len;

    char serv_addr_str[100];
    char buffer[10000];

    /* connect to the server */
    printf("Please enter a username:"); // get a username from the user
    bzero(username,16);
    fgets(username,15,stdin);
    buf_len=strlen(username);
    
    sockfd=socket(AF_INET,SOCK_STREAM,0); //create socket
    if (sockfd<0)
    {printf("Error creating socket\n");exit(1);}
    else
    {printf("Socket is successfully created\n");}
 
    	printf("Enter the port number:\n");
	scanf("%d",&portno);

	printf("Enter the IP Address\n");
	scanf("%s",serv_addr_str);
	inet_pton(AF_INET, serv_addr_str, &(serv_addr.sin_addr));

    memset(&serv_addr,'\0',sizeof(serv_addr)); //zero structure out
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port=htons(portno);

    connfd=connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr));
    if (connfd< 0)
    {printf("Error connecting\n");}
    else
    {printf("Client is connected to the server\n");}
    
    
    joinServer(username,sockfd,buf_len);
   
    //Clear the socket set
    FD_ZERO(&readfd);
    FD_SET(0,&readfd);
    FD_SET(sockfd,&readfd);

     while(1)
     {
         fdmax=sockfd;
         if (select(fdmax+1,&readfd,NULL,NULL,NULL)<0)
        {
	perror("Select failed\n");exit(1);
	}
        else
         {
         if (FD_ISSET(sockfd,&readfd))
        	{
		recv(sockfd,buffer,2000,0);
		rcv_msg(buffer);
		}
        
         if (FD_ISSET(0,&readfd))
         	{
		sendMessage(sockfd);
	 	}
	
	FD_SET(0,&readfd);
	FD_SET(sockfd,&readfd);
        } 
     }
        
    close(sockfd);
    printf("Client close\n");
    return 0;
   
}
