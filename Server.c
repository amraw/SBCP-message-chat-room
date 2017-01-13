//Insert Header Files
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>

//Insert Defined Keywords
#define JOIN 2
#define SEND 4
#define FWD 3

typedef struct		//Header File 
{
	int8_t vrsn;	
	int8_t type;
	int16_t len;
}Msg_header;

typedef struct		//Attribute Field
{
	int16_t type;
	int16_t len;
	char payload[511];
}Msg_attribute;

typedef struct		//Combine Header and Attribute Fields
{
	Msg_header header;
	Msg_attribute attribute;
}message;

//insert send to all function
void send_all(char *msg_tf,int port_excptn,int server_sd,int max_sd)
{
int f;
for(f=0;f<=max_sd;f++)
	{
	if (f !=port_excptn && f!=server_sd)	//Do not send to original user and server
		{
		if(send(f,msg_tf,1000,0),0<0)
			{
			printf("Error sending fwd message"); 
			}
		} 
	}
}

message msg;

int rcv_msg(char *buffer)     //decode message from buffer
{
		int msg_header_vrsn; 
		int msg_header_type;
		int msg_header_Msg_len;
		int msg_attribute_type;
		int msg_attribute_len;
		char msg_attribute_payload[511];
		sscanf(buffer,"%[^:]:%[^:]:%[^:]:%[^:]:%[^:]:%s", &msg_header_vrsn, &msg_header_type,&msg_header_Msg_len,&msg_attribute_type,&msg_attribute_len,msg_attribute_payload); //read in ascii value from buffer	
		msg.header.vrsn=msg_header_vrsn-'0'; //convert from ascii value into int
		msg.header.type=msg_header_type-'0';
		msg.header.len=msg_header_Msg_len-'0';
		msg.attribute.type=msg_attribute_type-'0';
		msg.attribute.len=msg_attribute_len-'0';
		strncpy(msg.attribute.payload,msg_attribute_payload,511);
		printf("\n Message Received \n");
	return(0);
}

int main()
{  
	//Declare socket variables
	int serv_sock, new_sock,port_num;
	socklen_t length_client;
	struct sockaddr_in serv_addr, client_addr;
	
	//Declare message varibales
	message msg_f;

	//Declare variables used for loop control
	int max_clients,n,rw_check,i,master_check,client_count;
	master_check=0; 
	client_count=0;

	//Declare error message variables
	char user_err_msg[200];	
	strcat(user_err_msg,"User Name is already taken, try joining with a different name");	
	
	//Declare FD_Set Variables
	fd_set read_fds;
	fd_set read_new;
	FD_ZERO(&read_fds);
	FD_ZERO(&read_new);

	//Buffer Variables
	char buffer[1000];
	int buffer_size=sizeof(buffer);
	char IP_Address[100];

	//take user inputs for IP Address, Port Num, and Maximum Clients
	printf("Enter the IP Address:\n");
	scanf("%s",IP_Address);
	
	printf("Enter the port number:\n");
	scanf("%d",&port_num);

	printf("Enter the max number of clients:\n");
	scanf("%d",&max_clients);

	char user_names[max_clients][511];

	memset(&serv_addr, 0, sizeof(serv_addr)); 

	//open socket
	serv_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(serv_sock<0)
		{
		printf("Error Attempting to Open Socket");
		}
	printf("Socket Created\n"); 

	//allow multiple connections
		int opt=1; 
		if( setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,sizeof(opt))<0)
		{
		printf("Error allowing multiple connections on server socket\n");	
		}
		
	//bind socket
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(port_num);	
		if(bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <0)
		{
		printf("Error Attempting to Bind Socket\n"); 
		perror("Error"); 
		}
	printf("Socket is Bound and Listening\n");

	//listen for client
	listen(serv_sock,max_clients);
	printf("Listen complete\n");

	//Constants used in loop 
	length_client = sizeof(client_addr);	
	n=serv_sock;  //n will be used to compute largest fds
			
	FD_SET(serv_sock,&read_new); //add server socket to FD list

	//Enter While Loop	 
	master_check=1;

	while(master_check)
	{
		//Update FD Set
		read_fds=read_new;
				
		rw_check = select (n+1,&read_fds, NULL, NULL, NULL); //replace NULL with timeout value if desired 
		if(rw_check < 0) 
		{
		printf("Error In Select Function\n");
		}
		
		for(i=0; i<=n;i++)  //Iterate through FD Set to see if any data rcv'd
		{
			if(FD_ISSET(i, &read_fds))	//Data received on the ith connection
			{
				if(i==serv_sock) //Data received on main server so client trying to connect
				{
					if(client_count<(max_clients+1))
					{
					new_sock = accept(serv_sock, (struct sockaddr *) &client_addr, &length_client); //Accept Client 
						if(new_sock<0)
						{
						printf("Error Accepting Socket\n");
						}
						else
						{
						client_count=client_count+1; 
						FD_SET(new_sock, &read_new); //add to FD set
							if(new_sock>n)
							{
							n=new_sock; //new largest fds 
							}
						printf("socket accepted\n");
						} 
					}
					else
					{
					printf("Client cannot be added because chat room is full\n");
					}			
				}
				else  //Data on Client Port
				{
					int recv_check=recv(i,buffer,buffer_size,0);
					if(recv_check<=0) //Client has exited
					{ 
					
					//Remove Client, Clean Resources and Inform Other Clients
					FD_CLR(i,&read_new);
					
					//Remove Client Name from list
					close(i);
					client_count=client_count-1;
					printf("Client has exited\n");
					//insert function to tell all clients					
					}
					else //Message Received
					{
					//Insert Receive Function
					rcv_msg(buffer);
						if(msg.header.type==2 && msg.attribute.type==2)
						{
						int m,username_check;
						username_check=0; 
							for(m=0;m<=max_clients;m++)
							{
								if(strcmp(user_names[m],msg.attribute.payload)==0) //Compare to list of user names
								{	
															
									if(send(i,user_err_msg,sizeof(user_err_msg),0))
									{
									printf("Error sending username error to client");
									}
									client_count=client_count-1; //alert user of error and cleanup resources
									close(i);
									FD_CLR(i,&read_fds);
									username_check=1;
									break;
								}
							}
							if (username_check==0)
							{
							printf("User %s has joined the chat room",msg.attribute.payload);
							sprintf(user_names[i],"%s",msg.attribute.payload); 
							} 
						}
						if(msg.header.type==4 && msg.attribute.type==4) //Check if Forward 
						{
						memset(&msg_f.attribute.payload[0],0,sizeof(msg_f.attribute.payload));
						strcat(msg_f.attribute.payload,"3:3:X:4:X:"); 
						strcat(msg_f.attribute.payload,user_names[i]);
						strcat(msg_f.attribute.payload,":");
						strcat(msg_f.attribute.payload,msg.attribute.payload); 
						send_all(msg_f.attribute.payload,i,serv_sock,n);
						}		
					}
				}
			
			}  //end of incoming data
		} //FD iteration 
	} //end of while loop
printf("Exiting program\n"); 
return 0; 
}//end of int main
