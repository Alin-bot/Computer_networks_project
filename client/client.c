#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
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

    strtok(letter, "\n"); // for enter
}

// read from player input a word
void chooseWord(char* word)
{
    // input a word
    printf("Choose a word with your letter: ");
    bzero(word, 50);
    fflush(stdout);

    read(0, word, 50);

    strtok(word, "\n"); // for enter
}

// read from player input a word with 
void chooseWordBasedOnLetters(char* word, char* letters)
{
    // input a word
    printf("Choose a word with the letters (%c%c): ", letters[0], letters[1]);
    bzero(word, 50);
    fflush(stdout);

    read(0, word, 50);

    strtok(word, "\n"); // for enter
}

int main ()
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
		perror ("[-]Error at connect().\n");
		return errno;
	}

	printf("[+] Connected to the server successfully!\n");
	printf("[+] The game will start when all the players are connected!\n");

    int OK = 0;
    int order_number;
    char word[50];
    char letter;
    char letters[3];

    while(OK == 0)
	{
		while(OK == 0)
		{
            // read order number
            order_number = readNumber(sd);

            if(order_number == 999)
            {
                printf("[] You won!\n");
                return 0;
            }

            // Playing first round
            printf("[1] Playing first round.\n");

            bzero(word, 50);

            if(order_number == 0) // player needs to give a letter + word
            {
                // chose letter and send to server
                chooseLetter(&letter);
                printf("The letter you chose is: %c\n", letter);

                sendLetter(&letter, sd);

                // chose word and send to server
                chooseWord(word);
                printf("The word you chose is: %s\n", word);

                sendWord(word, sd);
            }
            else // other players will wait
            {
                printf("A player is choosing a word, please wait %d more rounds..\n", order_number);

                // read letter
                readLetter(&letter, sd);
                printf("The player chose the letter: %c\n", letter);

                // read word
                readWord(word, sd);
                printf("The player chose the word: %s\n", word);
            }

            // if word was good / word was bad - based on the order number
            OK = readNumber(sd);
			if(OK == 1)
			{
				printf("[+] The word was correct! The next player will continue..\n");
			}
			else if(OK == 0 && order_number == 0)
			{
				printf("[-] You chose a wrong word, you will be disconnected!\n");
				OK = 2;
			}
			else if(OK == 0)
			{
				printf("[+] The word was wrong. The other player will continue..\n");
			}
		}

		while(OK == 1)
		{
            bzero(letters, 3);

            // getting the last 2 letters of the word
            getLastTwoLetters(word, letters);

            // Playing next rounds after first round
            printf("[2] Playing/Continue game after first round.\n");

            // read order number
            order_number = readNumber(sd);
            bzero(word, 50);

            if(order_number == 0) // player needs to give a letter + word
            {
                // chose word and send to server
                chooseWordBasedOnLetters(word, letters);
                sendWord(word, sd);
            }
            else // other players will wait
            {
                printf("A player is choosing a word, please wait %d more rounds..\n", order_number);

                // read word
                readWord(word, sd);
                printf("The player chose the word: %s\n", word);
            }

			// if word was good / word was bad - based on the order number
            OK = readNumber(sd);
			if(OK == 1)
			{
				printf("[+] The word was correct! The next player will continue..\n");
			}
			else if(OK == 0 && order_number == 0)
			{
				printf("[-] You chose a wrong word, you will be disconnected!\n");
                OK = 2;
			}
			else if(OK == 0)
			{
				printf("[+] The word was wrong. The other player will continue..\n");
			}
		}
	}
    close(sd);
    return 0;
}