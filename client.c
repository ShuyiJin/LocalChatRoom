#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define ONLINE 1
#define OFFLINE 0
#define CHAT 2
#define MAX 128

typedef struct
{
	int type;
	char buf[MAX];
}msg_t;

void *receive_msg(void *p)
{
	int sockfd = *(int *)p;
	msg_t msg;

	while(1)
	{
		bzero(&msg, sizeof(msg));
		recvfrom(sockfd, &msg, sizeof(msg), 0, NULL, NULL);
		printf("received: %s", msg.buf);
	}
}
	
int main(int argc, const char *argv[])
{
	if(argc < 4)
	{
		printf("%s <port_client> <ip_server> <port_server>\n", argv[0]);
		return -1;
	}

	int myport = atoi(argv[1]);
	int toport = atoi(argv[3]);

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in myaddr, toaddr;

	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(myport);
	myaddr.sin_addr.s_addr = INADDR_ANY;

	toaddr.sin_family = AF_INET;
	toaddr.sin_port = htons(toport);
	toaddr.sin_addr.s_addr = inet_addr(argv[2]);

	if(bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind");
		return -1;
	}

	pthread_t id;
	pthread_create(&id, NULL, receive_msg, &sockfd);

	// ack server online
	msg_t mymsg = {ONLINE, "login\n"};
	sendto(sockfd, &mymsg, sizeof(msg_t), 0, \
			(struct sockaddr *)&toaddr, sizeof(struct sockaddr)); 


	// send message
	while(1)
	{
		bzero(&mymsg, sizeof(mymsg));
		fgets(mymsg.buf, sizeof(mymsg.buf), stdin);

		if(!strncmp(mymsg.buf, "Bye", 3))
		{
			mymsg.type = OFFLINE;
			sendto(sockfd, &mymsg, sizeof(mymsg), 0, \
					(struct sockaddr *)&toaddr, sizeof(struct sockaddr));
			exit(0);
		}
		else
		{
			mymsg.type = CHAT;
			int x = sendto(sockfd, &mymsg, sizeof(mymsg), 0, \
					(struct sockaddr *)&toaddr, sizeof(struct sockaddr));
		}
	}

	return 0;
}
