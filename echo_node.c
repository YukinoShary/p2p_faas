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
#include <fcntl.h>
#include <stdio.h>
#define MaxConnection 10
#define BUFFERSIZE 4096

struct sockaddr_in server;
char *buffer[BUFFERSIZE];
extern int echo_client (char *server, int portno);
int echo_func(int sock);

int main (int argc, char *argv[])
{
  char *server;
  int portno;
  if (argc != 3) 
  {
    fprintf(stdout, "Usage: %s host port.\n", argv[0]);
    exit(-1);
  }
  server = argv[1];
  printf("server %s\n", server);
  portno = strtol(argv[2], 0, 10);
  printf("portno %d\n", portno);
  echo_client(server, portno);
}

int echo_client(char *server, int portno)
{
  int sock;
  char sbuf[BUFFERSIZE];
  char rbuf[BUFFERSIZE];
  
  sock = tcp_connect (server, portno);
  if (sock < 0) {
    exit (-1);
  }
  echo_func(sock);
  close(sock);
}

int tcp_connect (char *server, int portno)
{
  struct addrinfo hints, *ai;
  struct sockaddr_in addr; /* sockaddr_storage */
  
  int s;
  int err;
  
  if (( s = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return (-1);
  }
  printf("socket() called\n");
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  
  if ((err = getaddrinfo(server, NULL, &hints, &ai))) {
    fprintf(stderr, "sockaddr too large (%d) > (%d)\n",
            ai->ai_addrlen, sizeof(addr));
    
    freeaddrinfo(ai);
    close(s);
    return(-1);
  }
  printf("getaddrinfo() called\n");
  
  memcpy (&addr, ai->ai_addr, ai->ai_addrlen);
  addr.sin_port = htons(portno);
  freeaddrinfo(ai);
  
  printf("freeaddrinfo() called\n");
  
  if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror(server);
    close(s);
    return (-1);
  }
  
  printf("connect() called\n");
  return(s);
}

int echo_func(int sock)
{
    ssize_t len;
    while(len = recv(sock, buffer, BUFFERSIZE, MSG_WAITALL) > 0)
    {
        send(sock, buffer, len, 0);
    }
    return 0;
}
