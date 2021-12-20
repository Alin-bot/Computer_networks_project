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
#include "../common_resources/functions.h"

// the port and the address
#define PORT 2024
#define ADDR "127.0.0.1"

extern int errno;

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
void chooseWordBasedOnLetters(char* word, char* letters)
{
  // input a word
  printf("Choose a word with the letters (%s): ", letters);
  bzero(word, 100);
  fflush(stdout);

  read(0, word, 100);
}

void playFirstRound(int sd, char* word, int number)
{
	char letter;	

	printf("a");
	number = readNumber(sd);
	printf("b");
	if(number == 0) // player needs to give a letter + word
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
	else // other players will wait
	{
		printf("A player is choosing a word, please wait %d more rounds..\n", number);

	    // read letter
		readLetter(sd, letter);
		printf("The player chose the letter: %s", letter);

		// read word
		readWord(sd, word);
		printf("The player chose the word: %s", word);
	}
}

void playNormalRounds(int sd, char* word, char* letters, int number)
{
	number = readNumber(sd);

	if(number == 0) // player needs to give a letter + word
	{
		// chose word and send to server
		chooseWordBasedOnLetters(word, letters);
		sendWord(sd, word);
	}
	else // other players will wait
	{
		printf("A player is choosing a word, please wait %d more rounds..\n", number);

		// read word
		readWord(sd, word);
		printf("The player chose the word: %s", word);
	}
}

int main (int argc, char *argv[])
{
	int sd;			// socket descriptor
	struct sockaddr_in server;	// the structure for the server where we will connect

	// creating the socket
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror ("[-] Error at socket().\n");
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

	printf("[+] Connected to the server succesfully!\n");
	printf("[+] The game will start when all the players are connected!\n");

	while(1)
	{
		int OK = 0;
		char word[100];
		int number;

		while(OK == 0)
		{
			playFirstRound(sd, word, number);

			// if word was good
			int OK = readNumber(sd);
			if(OK == 1)
			{
				printf("[+] The word was correct! The next player will continue.\n");
			}
			else if(OK == 0 && number == 0)
			{
				printf("[-] You chose a wrong word, you will be disconected!\n");
				close(sd);
			}
			else if(OK == 0 && number != 0)
			{
				printf("[+] The word was wrong. The other player will continue.\n");
			}
			else if(OK == 2)
			{
				printf("[] You won!\n");
			}
		}

		while(OK == 1)
		{
			char letters;
            strcat(letters, word[strlen(word) - 2]);
            strcat(letters, word[strlen(word) - 1]); // get the last 2 letters of the last word

			playNormalRounds(sd, word, letters, number);

			// if word was good
			int OK = readNumber(sd);
			if(OK == 1)
			{
				printf("[+] The word was correct! The next player will continue.\n");
			}
			else if(OK == 0 && number == 0)
			{
				printf("[-] You chose a wrong word, you will be disconected!\n");
				close(sd);
			}
			else if(OK == 0 && number != 0)
			{
				printf("[+] The word was wrong. The other player will continue.\n");
			}
			else if(OK == 2)
			{
				printf("[] You won!\n");
			}
		}
	}
}