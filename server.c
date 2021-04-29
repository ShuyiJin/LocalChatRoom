#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ONLINE 1
#define OFFLINE 0
#define CHAT 2
#define MAX 128

typedef struct
{
	int type;
	char buf[MAX];
}msg_t;

struct sockaddr_in client[MAX];
int number;

struct sockaddr_in myaddr, fromaddr;

void print_clients()
{
	printf("count = %d\n", number);
	int i;
	for(i = 0; i < number; i++)
	{
		printf("%s::%d\n", inet_ntoa(client[i].sin_addr), \
				client[i].sin_port);
	}
}
void addClient(struct sockaddr_in c)
{
	client[number] = c;
	number++;
	print_clients();
}

int cmpSockaddr(struct sockaddr_in a, struct sockaddr_in b)
{
	return (a.sin_port == b.sin_port) && \
						(a.sin_addr.s_addr == b.sin_addr.s_addr) ? 1:0;
}

int getIndex(struct sockaddr_in c)
{
	int i;
	for(i = 0; i < number; i++)
	{
		if(cmpSockaddr(c, client[i]))
			return i;
	}

	return -1;
}

void deleteClient(struct sockaddr_in c)
{
	int i = getIndex(c);
	if(i < 0)
	{
		printf("error deleteClient");
		return;
	}
	
	number--;	
	while(i < number)
	{
		client[i] = client[i+1];
		i++;
	}		
}

void send_info(int sockfd, msg_t *msg)
{
	int i;
	for(i = 0; i < number; i++)
	{
		if(cmpSockaddr(fromaddr, client[i]))
			continue;
		sendto(sockfd, msg, sizeof(msg_t), 0, \
				(struct sockaddr *)&client[i], \
				sizeof(struct sockaddr));
	}

}
	
int main(int argc, const char *argv[])
{
	if (argc < 3)
	{
		printf("%s <ip> <port> \n", argv[0]);
		return -1;
	}

	int myport = atoi(argv[2]);
	char myip[16];
	strcpy(myip, argv[1]);
		
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(myport);
	//myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_addr.s_addr = inet_addr(myip);


	if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind");
		return -1;
	}
	
	msg_t msg;
	socklen_t len = sizeof(struct sockaddr);
	char tempbuf[MAX];

	while(1)
	{
		bzero(&fromaddr, sizeof(fromaddr));
		bzero(&msg, sizeof(msg));

		ssize_t x = recvfrom(sockfd, &msg, sizeof(msg), 0, \
				(struct sockaddr *)&fromaddr, &len);

		sprintf(tempbuf, "addr: %s, port: %hd, info: %s", \
				inet_ntoa(fromaddr.sin_addr), \
				fromaddr.sin_port, msg.buf);
		strcpy(msg.buf, tempbuf);

		if(msg.type == ONLINE/*online*/)
		{
			printf("online +1\n");
			//record client
			addClient(fromaddr);
			send_info(sockfd, &msg);
			printf("online: %d\n", number);
		}
		else if(msg.type == OFFLINE/*offline*/)
		{
			//remove client
			deleteClient(fromaddr);
			send_info(sockfd, &msg);
			printf("online: %d\n", number);
		}
		else if(msg.type == CHAT/*chat*/)
		{
			send_info(sockfd, &msg);
		}
	}
	return 0;
}
