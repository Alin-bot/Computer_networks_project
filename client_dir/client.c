#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>


// the port and the address
#define PORT 2024
#define ADDR "127.0.0.1"

extern int errno;

int main (int argc, char *argv[])
{
  int sd;			// socket descriptor
  struct sockaddr_in server;	// the structure for the server where we will connect

  // creating the socket
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror ("[-]Error at socket().\n");
    return errno;
  }

  // socket family
  server.sin_family = AF_INET;
  // server i address
  server.sin_addr.s_addr = inet_addr(ADDR);
  // the port
  server.sin_port = htons (PORT);

  // connecting to the server
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
    perror ("[-]Eroare la connect().\n");
    return errno;
  }

  printf("Connected to the server succesfully!");

  char msg[255];
  if (read (sd, msg, 100) < 0)
  {
    perror ("[-]Eroare la read() de la server.\n");
    return errno;
  }




  printf("[-]Game done!\n");

  // closing the socket descriptor
  close (sd);
}
