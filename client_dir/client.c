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

// read a number from socket descriptor and return it
int readOrder(int sd)
{
  int order;
  fflush(stdout);

  if (read (sd, &order, sizeof(int)) < 0)
  {
    perror ("[-]Error at read().\n");
    return errno;
  }

  return order;
}

// read from player input a letter
void chooseLetter(char* letter)
{
  // input a letter
  printf("You are the first player! Choose a letter: ");
  bzero(letter, 2);
  fflush(stdout);

  read(0, letter, 2);
}

// read from player input a word
void chooseWord(char* word)
{
  // input a word
  printf("Choose a word with your letter: ");
  bzero(word, 100);
  fflush(stdout);

  read(0, word, 100);
}

// read from player input a word with 
void chooseWordBasedOnLetters(char* letters, char* word)
{
  // input a word
  printf("Choose a word with the letters (%s): ", letters);
  bzero(word, 100);
  fflush(stdout);

  read(0, word, 100);
}

// send a letter to socket descriptor 
void sendLetter(int sd, char* letter)
{
  // send the letter to the server
  if (write(sd, letter, 2) <= 0)
  {
    perror("[-]Error at write().\n");
  }
}

// send a word to socket descriptor
void sendWord(int sd, char* word)
{
  // send the word to the server
  if (write(sd, word, 100) <= 0)
  {
    perror("[-]Error at write().\n");
  }
}

// read a letter from socket descriptor
void readLetter(int sd, char* letter)
{
  bzero(letter, 2);
  fflush(stdout);

  // read the letter
  if (read (sd, letter, 2) < 0)
  {
    perror ("[-]Error at read().\n");
    return errno;
  }
}

// read word from socket descriptor
void readWord(int sd, char* word)
{
  bzero(word, 100);
  fflush(stdout);

  // read the word
  if (read (sd, word, 100) < 0)
  {
    perror ("[-]Error at read().\n");
    return errno;
  }
}

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

  printf("[+]Connected to the server succesfully!\n");



  while(1)
  {
    char letter[2];
    char word[100];

    int order = readOrder(sd);
    printf("Your order number is: %d\n", order);

    // if first player of first round - else other players
    if(order == 0)
    {
      // chose letter and send to server
      chooseLetter(letter);
      printf("The letter you chose is: %s", letter);
      sendLetter(sd, letter);
      printf("[+]The letter was send!\n");

      // chose word and send to server

      chooseWord(word);
      sendWord(sd, word);
    }
    else 
    {
      printf("A player is choosing a word, please wait %d more rounds..\n", order);

      // read letter
      readLetter(sd, letter);
      printf("The player chose the letter: %s", letter);

      // read word
      readWord(sd, word);
      printf("The player chose the word: %s", word);
    }
/*
    int response;

    // read the response (if all is good)
    if (read(sd, response, sizeof(int)) < 0)
    {
      perror ("[-]Error at read().\n");
      return errno;
    }

    if(response == 0)
    {
      printf("You'll be kicked out!");
      break;
    }*/
  }



  printf("[+]Game done!\n");

  // closing the socket descriptor
  close (sd);
}
