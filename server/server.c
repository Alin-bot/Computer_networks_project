#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../common_resources/functions.h"

// the port
#define PORT 2024

extern int errno;

// function that reads a number from a given file path
int readMaxNumberOfClients(char* file_name)
{
    FILE *fp;
    fp = fopen(file_name, "r"); // opne file

    char file_content[255];
    fscanf(fp, "%s", file_content);

    fclose(fp);
    return atoi(file_content);
}

// function that accepts clients to the server and give the list of socket descriptors
int getPlayersList(int sd, struct sockaddr_in from, int max_number_of_players, int list[max_number_of_players])
{
    int length = sizeof(from);

    for(int i = 0; i < max_number_of_players; i++)
    {
        printf("[+] Waiting for player number %d.\n", i + 1);
        list[i] = accept(sd, (struct sockaddr *)&from, &length);

        if (list[i] < 0)
        {
            perror("[-] Error at accepting the client.\n");
            continue;
        }
    }
}

// function that closes all the socket descriptors from list
void closePlayersList(int max_number_of_players, int list[max_number_of_players])
{
    for (int i = 0; i < max_number_of_players; ++i)
    {
        close(list[i]);
    }
}

// function that see if word is in dictionary
int wordIsGood(char* word, char* letter)
{
    return 1;
}

void playFirstRound(int max_number_of_players, int players_list[max_number_of_players], char* word, char* letter)
{
    // give players the order number to know when is their turn
    for (int i = 0; i < max_number_of_players; ++i)
    {
        sendNumber(players_list[i], i);
    }

    printf("stopped to read letter\n");
    // read the letter of the first player
    readLetter(players_list[0], letter);

    // send the letter to the other players
    for (int i = 1; i < max_number_of_players; ++i)
    {
        sendLetter(players_list[i], letter);
    }

    // read the word of the first player
    readWord(players_list[0], word);

    // send the word to the other players
    for (int i = 1; i < max_number_of_players; ++i)
    {
        sendWord(players_list[i], word);
    }
}

void playNormalRounds(int max_number_of_players, int players_list[max_number_of_players], char* word)
{
    // give players the order number to know when is their turn
    for (int i = 0; i < max_number_of_players; ++i)
    {
        sendNumber(players_list[i], i);
    }

    // read the word of the first player
    readWord(players_list[0], word);

    // send the word to the other players
    for (int i = 1; i < max_number_of_players; ++i)
    {
        sendWord(players_list[i], word);
    }
}

int main()
{
    int sd; // socket descriptor

    // creating the socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[-] Error at the socket() function.\n");
        return errno;
    }

    // for bind problems on forcing close server
    int enable = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed\n");
    
    struct sockaddr_in server; // server structure

    // getting ready the server structure
    bzero(&server, sizeof(server));

    // socket family
    server.sin_family = AF_INET;
    // accepting all addresses
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    // what port we are using
    server.sin_port = htons(PORT);

    // attaching IP address and the port to the socket
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[-] Error at the bind() function.\n");
        return errno;
    }

    // listen for the client so we can accept them
    if (listen(sd, 1) == -1)
    {
        perror("[-] Error at the listen() function.\n");
        return errno;
    }

    struct sockaddr_in from; // client structure

    // getting ready the client structure
    bzero(&from, sizeof(from));

    // reading max number of players
    int max_number_of_players = readMaxNumberOfClients("config_file.txt");

    while(1)
    {
        printf("[+] Waiting on the port %d...\n", PORT);
        fflush(stdout);

        int players_list[max_number_of_players]; // list of socket descriptors of players

        // getting the list of players for the current game
        getPlayersList(sd, from, max_number_of_players, players_list);

        int pid;
        
        // making a process for the game
        if ((pid = fork()) == -1) // error at fork
        {
            closePlayersList(max_number_of_players, players_list);
            continue;
        }
        else if (pid > 0) // main process
        {
            // we dont need the socket descriptors for the clients
            closePlayersList(max_number_of_players, players_list);

            while (waitpid(-1, NULL, WNOHANG))
                ;
            continue;
        }
        else if (pid == 0) // game process
        {
            // we dont need server sd
            close(sd);

            printf("[+] A game has started!\n");

            while(1)
            {
                printf("[+] A game has started!\n");
                int OK = 0;
                char word[100];
                char letter[2];

                while(OK == 0)
                {
                    if(max_number_of_players == 1)
                    {
                        sendNumber(players_list[0], 2); // won because last player
                        exit(0);
                    }

                    playFirstRound(max_number_of_players, players_list, word, letter);

                    // if the word is good, anounce players
                    if(wordIsGood(word, letter))
                    {
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            sendNumber(players_list[i], 1);
                        }
                        OK = 1;
                    }
                    else // if the player chose a wrong word
                    {   
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            sendNumber(players_list[i], 1);
                        }

                        close(players_list[0]);

                        // edit the list of socket descriptors of clients
                        for (int i = 1; i < max_number_of_players; ++i)
                        {
                            players_list[i-1] = players_list[i];
                        }
                        max_number_of_players--;
                    }
                }

                while(OK == 1)
                {
                    if(max_number_of_players == 1)
                    {
                        sendNumber(players_list[0], 2); // won because last player
                        exit(0);
                    }

                    strcat(letter, word[strlen(word) - 2]);
                    strcat(letter, word[strlen(word) - 1]); // get the last 2 letters of the last word

                    playNormalRounds(max_number_of_players, players_list, word);

                    // if the word is good, anounce players
                    if(wordIsGood(word, letter))
                    {
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            sendNumber(players_list[i], 1);
                        }
                    }
                    else // if the player chose a wrong word
                    {   
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            sendNumber(players_list[i], 1);
                        }

                        close(players_list[0]);

                        // edit the list of socket descriptors of clients
                        for (int i = 1; i < max_number_of_players; ++i)
                        {
                            players_list[i-1] = players_list[i];
                        }
                        max_number_of_players--;
                        OK = 0;
                    }
                }

                


            }

            printf("[+]Game done.\n");

            closePlayersList(max_number_of_players, players_list);

            exit(0);

        }

        bzero(players_list, max_number_of_players);
    }
}