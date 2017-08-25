#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

// the port client will be connecting to
#define PORT 5709


// max number of bytes we can get at once
#define MAXDATASIZE 300
 
void readLine(char *line) 
{
	int c, i = 0; 
	for(; (c = getchar()) != '\n'; i++) line[i] = c;
	line[i] = '\0';
}

int main(int argc, char *argv[])
{
    //argv[1] = "andromeda-51.ics.uci.edu";
    char * username = argv[1];
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct hostent *he;
    // connector’s address information
    struct sockaddr_in their_addr;
     
    // if no command line argument supplied
    if(argc != 2)
    {
        fprintf(stderr, "Client-Usage: %s the_client_hostname\n", argv[0]);
        // just exit
        exit(1);
    }
     
    // get the host info
    if((he=gethostbyname("andromeda-40.ics.uci.edu")) == NULL)
    {
        perror("gethostbyname()");
        exit(1);
    }
    else
        printf("Client-The remote host is: %s\n", "andromeda-51.ics.uci.edu");
     
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket()");
        exit(1);
    }
    else
        printf("Client-The socket() sockfd is OK...\n");
     
    // host byte order
    their_addr.sin_family = AF_INET;
    // short, network byte order
    printf("Server-Using %s and port %d...\n", "andromeda-51.ics.uci.edu", PORT);
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    // zero the rest of the struct
    memset(&(their_addr.sin_zero), '\0', 8);

     
    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect()");
        exit(1);
    }
    else
        printf("%s %s\n", username, "has joined the chatroom" );
        printf("Client-The connect() is OK...\n");
     
/*    if((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
    {
        perror("recv()");
        exit(1);
    }
    else
        printf("Client-The recv() is OK...\n");
     
    buf[numbytes] = '\0';
    printf("Client-Received: %s", buf);*/
    fd_set active_fd_set, read_fd_set; 
    FD_ZERO(&active_fd_set);
    FD_SET(sockfd, &active_fd_set); 
    FD_SET(0, &active_fd_set); 

    char msgbuf[MAXDATASIZE];
    sprintf(msgbuf, "ENTER %s", username);
    write(sockfd, msgbuf, MAXDATASIZE);
	
	while(1)
	{
	/*	char line[MAXDATASIZE];
		readLine(line);
		sprintf(msgbuf, "SAY %s %s", username, line);
		write(sockfd, msgbuf, MAXDATASIZE);
	*/	
		int i;
		read_fd_set = active_fd_set;
		if(select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) 
		{ perror("select"); }
		
		char buf[MAXDATASIZE];
		// service sockets with input pending
		for(i = 0; i < FD_SETSIZE; i++)
			if(FD_ISSET(i, &read_fd_set)) 
			{
				if(i==sockfd) 
				{
					read(sockfd, buf, MAXDATASIZE);	
					printf(buf);
				}	
				else
				{
					//read(0, buf, MAXDATASIZE);	
					readLine(buf);
					sprintf(msgbuf, "SAY %s %s", username, buf);
					write(sockfd, msgbuf, MAXDATASIZE);
				}
			}
			
		
	}

    write(sockfd, "leaving chatroom", MAXDATASIZE);
    close(sockfd);
    return 0;
}
