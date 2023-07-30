#include <stdio.h>
#include <errno.h>
#include <sys/types.h>   /* socket() */
#include <sys/socket.h>  /* socket() */
#include <netinet/in.h>  /* struct sockaddr_in */
#include <netdb.h>       /* getaddrinfo() */
#include <stdlib.h>      /* exit() */
#include <string.h>      /* memset */

#define BUFFERSIZE 4096
int send_client (char *server, char* filename, int portno);
int fdopen_sock(int sock, FILE **outp);
int tcp_connect (char *server, int portno);
int fdopen_file(char* filename, FILE** file_fd);

int main (int argc, char *argv[])
{
  char *server, *filename;
  int portno;
  if (argc != 4) {
    fprintf(stdout, "Usage: %s host port.\n", argv[0]);
    exit(-1);
  }
  server = argv[1];
  printf("server %s\n", server);
  portno = strtol(argv[2], 0, 10);
  printf("portno %d\n", portno);
  filename = argv[3];
  printf("filename %s\n", filename);
  send_client(server, filename, portno);
}

int send_client(char *server, char* filename, int portno)
{
  int sock, total_send;
  ssize_t send_size;
  char sbuf[BUFFERSIZE];
  FILE *out, *file_fd;
  
  sock = tcp_connect (server, portno);
  if (sock < 0) {
    exit (-1);
  }
  
  if(fdopen_file(filename, &file_fd) < 0){
    fprintf(stderr, "fdopen_file()\n");
    exit(1);
  }

  if (fdopen_sock(sock, &out) < 0) {
    fprintf(stderr, "fdopen()\n");
    exit(1);
  }

  /* wait for sending signal(data page) and clear buffer*/
  recv(sock, sbuf, BUFFERSIZE, 0);
  memset(sbuf, 0, BUFFERSIZE);

  /* gets buffer */
  printf("start to send\n");
  while (fread(sbuf, 1, BUFFERSIZE, file_fd)) 
  {
    send_size = send(sock, sbuf, BUFFERSIZE, 0);
    if(send_size == -1)
    {
      perror("error while sending data.\n");
      return -1;
    }
    else
    {
      total_send += send_size;
      printf("sending progress:%d/%d\n", total_send, 100 * (1 << 20));
    }
  }
  fclose(file_fd);
  fclose(out);
  return 0;
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

int fdopen_sock(int sock, FILE **outp)
{
  int sock2;
  if ((sock2 = dup(sock)) < 0)
  return (-1);
  printf("dup() called\n");
  
  if ((*outp = fdopen(sock, "w")) == NULL) {
    return (-1);
  }
  setvbuf(*outp, (char *) NULL, _IOLBF, 0);
  return(0);
}

int fdopen_file(char* filename, FILE** file_fd)
{
  *file_fd = fopen(filename, "r");
  if(*file_fd == NULL)
  {
    perror("file open failed.\n");
    return (-1);
  }
  fseek(*file_fd, 0, SEEK_SET);
  return 1;
}



