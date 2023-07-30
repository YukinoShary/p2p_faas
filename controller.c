#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#define MaxConnection 4
#define BUFFERSIZE 4096

typedef struct pipeline_ip
{
    struct node_ip *head;
    struct node_ip *rear;
} pipeline_ip;

typedef struct node_ip
{
    in_addr_t ip;
    int fd;
    struct node_ip *next;
} node_ip;

typedef struct active_soc
{
    int fd;
    in_addr_t ip;
} active_soc;

char *buffer;
struct pipeline_ip *ip_table[2]; //3 pipelines to be controlled
struct timeval *start_t, *end_t, *time_result;
int selectSolution(int sockfd);
int transfer_func(struct active_soc *atv_soc);
int send_start(active_soc *socs[]);
int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y);
int main(int argc, char *argv[])
{
    int sockfd, i;
    struct sockaddr_in server;
    struct node_ip *n0, *n1, *n2, *n3;

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

    //create the pipeline_ip table for data transfer 
    n0 = malloc(sizeof(node_ip));
    n1 = malloc(sizeof(node_ip));
    n2 = malloc(sizeof(node_ip));
    n3 = malloc(sizeof(node_ip));

    n0->ip = inet_addr("192.168.128.27");
    n1->ip = inet_addr("192.168.128.26");
    n2->ip = inet_addr("192.168.128.24");
    n3->ip = inet_addr("192.168.128.23");

    n0->next = n2;
    n1->next = n3;

    for(i = 0; i < 2; i ++)
    {
        ip_table[i] = malloc(sizeof(pipeline_ip));
    }
    ip_table[0]->head = n0;
    ip_table[0]->rear = n2;
    ip_table[1]->head = n1;
    ip_table[1]->rear = n3;

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (struct sockaddr*)&server, sizeof(server))) != 0) 
    {
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
    int i, j, maxfd, count, ret, finished; 
    struct sockaddr_in client;
    struct timeval* timeout;
    struct active_soc *atv_socs[MaxConnection];
    fd_set *readfds;
    socklen_t sin_siz;
    sin_siz = sizeof(client);
    for(i = 0; i < MaxConnection; i++)
    {
        atv_socs[i] = malloc(sizeof(active_soc));
        if((atv_socs[i] = malloc(BUFFERSIZE)) == NULL)
        {
            perror("failed to malloc() for atv_socs\n");
            return -1;
        }
    }
    if((buffer = malloc(BUFFERSIZE)) == NULL)
    {
        perror("failed to malloc() for buffer\n");
        return -1;
    }

    //set timeout
    timeout = malloc(sizeof(struct timeval));
    timeout->tv_sec = 30;
    timeout->tv_usec = 0;

    for(i = 0; i < MaxConnection - 1; i ++)
    {
        atv_socs[i]->fd = 0;
        atv_socs[i]->ip = client.sin_addr.s_addr;
    }

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
            if(atv_socs[j]->fd > 0)
            {
                maxfd = (maxfd > atv_socs[j]->fd) ? maxfd : atv_socs[j]->fd;
                FD_SET(atv_socs[j]->fd, readfds);
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
                if(FD_ISSET(atv_socs[j]->fd, readfds))
                {
                    timeout->tv_sec = 30;
                    timeout->tv_usec = 0;
                    if ((ret = transfer_func(atv_socs[j])) == 1)
                    {
                        //transfer finished
                        gettimeofday(end_t, NULL);
                        finished ++;
                    }
                }
            }
            if(FD_ISSET(sockfd, readfds)) 
            {                                                                  
                //accept new connection and set connection fd
                atv_socs[i]->fd = accept(sockfd, (struct sockaddr *)&atv_socs[i]->ip, &sin_siz);
                count ++;     
                if(count >= 4)
                {
                    send_start(atv_socs);
                }  
                timeout->tv_sec = 30;
                timeout->tv_usec = 0;      
            }
            if(finished >= 2)
                break;
        }      
    }

    if(timeval_subtract(time_result, end_t, start_t) == 1)
    {
        printf("ERROR during computing esplased time.\n");
    }
    else
    {
        printf("used time: %ld.%06ld\n", time_result->tv_sec, time_result->tv_usec);
    }
    free(start_t);
    free(end_t);
    free(time_result);
    return 0;
}

int transfer_func(struct active_soc *atv_soc)
{
    ssize_t len;
    int opposite_fd;
    //search the opposite node to transfer
    if(atv_soc->ip == ip_table[0]->head->ip)
    {
        opposite_fd = ip_table[0]->rear->fd;
    }
    else if(atv_soc->ip == ip_table[1]->head->ip)
    {
        opposite_fd = ip_table[1]->rear->fd;
    }
    else
    {
        return 1;
    }
    while((len = recv(atv_soc->fd, buffer, BUFFERSIZE, MSG_DONTWAIT) > 0))
    {
        send(opposite_fd, buffer, len, MSG_DONTWAIT);
    }
    return 0;
}

int send_start(active_soc* socs[])
{
    int i;
    start_t = malloc(sizeof(struct timeval));
    end_t = malloc(sizeof(struct timeval));
    time_result = malloc(sizeof(struct timeval));
    gettimeofday(start_t, NULL);
    memcpy(buffer, start_t, sizeof(start_t));
    for(i = 0; i < MaxConnection; i ++)
    {
        if(socs[i]->ip == ip_table[0]->head->ip 
        || socs[i]->ip == ip_table[1]->head->ip)
        {
            send(socs[i]->fd, buffer, sizeof(start_t), 0);
        }
    }
    printf("start to send data\n");
    return 0;
}

/* Subtract the ‘struct timeval’ values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0. */

int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}