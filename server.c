// server.c 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define PORT 5709
#define MAXDATASIZE 300

#define BACKLOG 10 

void printHostName()
{
	char hostName[126]; 
	gethostname(hostName, 126);
	printf("HOSTNAME: %s\n", hostName);
}

void sendToAll(char* msg, fd_set active_fd_set, int sockfd)
{
	int i; 
	for(i = 0; i < FD_SETSIZE; i++)
	{
		if(FD_ISSET(i, &active_fd_set) && i != sockfd)
			write(i, msg, MAXDATASIZE); 
	}
} 

int read_from_client(int fd, fd_set active_fd_set, int sockfd) 
{
	char buf[MAXDATASIZE];
	int nbytes = read(fd, buf, MAXDATASIZE);
	if(nbytes < 0) 
	{
		perror("read");
	}
	else if(nbytes == 0)
		return -1;
	else // data read
	{
		printf("got message: %s\n", buf);
		char msg[MAXDATASIZE];

		int c = buf[0];
		switch(c)	
		{
			case 'E':
				sprintf(msg, "%s has joined the chatroom\n", buf+6);
				sendToAll(msg, active_fd_set, sockfd);
				break;
			case 'S': ;
				char* user = strtok(buf+3, " ");	
				char* mptr = buf+3;
				while(*mptr++ != '\0');
				sprintf(msg, "%s: %s\n", user, mptr);
				sendToAll(msg, active_fd_set, sockfd);
				break;	
			case 'L': ;
				char* leaveUser = strtok(buf+5, " ");
				sprintf(msg, "%s has left the chatroom\n", leaveUser);		
				sendToAll(msg, active_fd_set, sockfd);
				break;
		}
		return 0; 
	}
}

int main() 
{
	int sockfd;
	fd_set active_fd_set, read_fd_set; 
	struct sockaddr_in my_addr; struct sockaddr_in their_addr;
	int sin_size;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) perror("socket err");	
	else printf("socket OK\n");
	
	memset(&my_addr, 0, sizeof my_addr);
	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr *) &my_addr, sizeof my_addr)==-1)
		perror("bind error"); 
	else printf("bind OK\n");
	if(listen(sockfd, BACKLOG) == -1) perror("listen error");
	else printf("listen OK\n");	

	FD_ZERO(&active_fd_set); 
	FD_SET(sockfd, &active_fd_set);

	while(1)
	{
		int i;
		read_fd_set = active_fd_set;
		if(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) 
		{ perror("select"); }
		
		// service sockets with input pending
		for(i = 0; i < FD_SETSIZE; i++)
			if(FD_ISSET(i, &read_fd_set)) 
			{
				if(i==sockfd) 
				{
					//connection request
					int new; 
					new = accept(sockfd, (struct sockaddr *)&their_addr,
							(socklen_t *)&sin_size);	
					if(new == -1) perror("accept"); 
					else printf("accept ok! \n");	
					FD_SET(new, &active_fd_set); 
				}	
				else
				{
					if(read_from_client(i, active_fd_set, sockfd) < 0) 
					{
						close(i);
						FD_CLR(i, &active_fd_set);	
					}
				}
			}
			
	}	
	/*
	client_fds[numClients] = accept(sockfd, (struct sockaddr *)&their_addr,
			(socklen_t *)&sin_size);
	if(client_fds[numClients] == -1) perror("accept error");	
	else printf("accept OK\n");

	write(client_fds[0], "welcome", 8);
	*/
}





