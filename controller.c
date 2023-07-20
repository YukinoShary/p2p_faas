#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MaxConnection 10


int selectSolution(int sockfd);
int main(int argc, char* argv[])
{
    int sockfd, connfd, len, connectionCount;
    char *filename;
    struct sockaddr_in server;

    //socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket creation failed...\n");
        return -1;
    }
    printf("socket successfully created..\n");
    int iSocketOption = 1;
    if( setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&iSocketOption, sizeof(iSocketOption)) < 0 )  
    {  
        perror("setsockopt");  
        return -1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(atoi(argv[1]));
    filename = argv[3];

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&server, sizeof(server))) != 0) {
        printf("socket bind failed...\n");
        return -1;
    }
    else
        printf("Socket successfully binded..\n");
    // Now server is ready to listen and verification
    if ((listen(sockfd, MaxConnection)) != 0) {
        printf("Listen failed...\n");
        return -1;
    }
    else
    {
        printf("Server listening..\n");
        selectSolution(sockfd);
    }
    printf("finish program\n");
    return 0;
}

int selectSolution(int sockfd)
{
    int i, j, maxfd, count; 
    char *buffer;
    FILE *file_fd;
    struct sockaddr_in client;
    struct timeval* timeout;
    struct fd_status **connfd;
    useconds_t time_result;
    fd_set *readfds;
    socklen_t sin_siz;
    sin_siz = sizeof(client);
    connfd = malloc(MaxConnection * sizeof(fd_status));
    for(i = 0; i < MaxConnection; i++)
    {
        connfd[i] = malloc(sizeof(fd_status));
        connfd[i]->fd = 0;
        connfd[i]->is_dispatched = false;
    }

    //set timeout
    timeout = malloc(sizeof(struct timeval));
    timeout->tv_sec = 30;
    timeout->tv_usec = 0;
    connectionCount = 0; //real time connection 

    //initialize readfds and set the sockfd
    readfds = malloc(sizeof(fd_set));
    i = 0;
    count = 0; //include finished connection
    
    while(1)
    {
        FD_ZERO(readfds); 
        //find new maxfd and initialize
        maxfd = 0;
        FD_SET(sockfd, readfds);
        for(j = 0; j < MaxConnection; j++)
        {
            if(connfd[j]->fd > 0)
            {
                maxfd = (maxfd > connfd[j]->fd) ? maxfd : connfd[j]->fd;
                FD_SET(connfd[j]->fd, readfds);
            }
        }
        if(maxfd == 0)
        {
            maxfd = sockfd;
        } 
        if((select(maxfd + 1, readfds, NULL, NULL, timeout)) > 0)
        {
            for(j = 0; j < MaxConnection; j++)
            {
                if(FD_ISSET(connfd[j]->fd, readfds) && !connfd[j]->is_dispatched)
                {
                    //recv ready
                    connfd[j]->is_dispatched = true;
                    timeout->tv_sec = 30;
                    timeout->tv_usec = 0;
                    receive_func(connfd[j]->fd, j, thpool, mode);  

                }
            }
            
            if(FD_ISSET(sockfd, readfds)) 
            {                                                                  
                //accept new connection and set connection fd
                connfd[i]->fd = accept(sockfd, (struct sockaddr *)&client, &sin_siz);               
                count ++;
                timeout->tv_sec = 30;
                timeout->tv_usec = 0;      
            }     
        }      
    }
    return 0;
}

int receive_func(int atv_sockfd, FILE* file)
{
    
}

int send_func(int atv_sockfd, FILE* file)
{

}