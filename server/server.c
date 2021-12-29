#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common_resources/functions.h"
#include <ctype.h>

// the port
#define PORT 2024

extern int errno;

// function that reads a number from a given file path
int readMaxNumberOfClients(char* file_name)
{
    FILE *fp;
    fp = fopen(file_name, "r"); // open file

    char file_content[255];
    fscanf(fp, "%s", file_content);

    fclose(fp);
    return atoi(file_content);
}

// function that accepts clients to the server and give the list of socket descriptors
void getPlayersList(int sd, struct sockaddr_in from, int max_number_of_players, int list[max_number_of_players])
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
int wordIsGood(char* word, char* letter, FILE *fp)
{
    printf("[?] Checking if word is good.. with the word: %s, and letter/letters %s\n", word, letter);

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    // upper the letters in word
    char new_word[100];
    bzero(new_word, 100);
    char ch;
    int i = 0;

    while (word[i]) {
        ch = toupper(word[i]);
        strcat(new_word, &ch);
        i++;
    }
    printf("New word in upper: %s\n", new_word);

    // upper the letters in letter
    char new_letters[3];
    bzero(new_letters, 3);
    i = 0;

    while (letter[i]) {
        ch = toupper(letter[i]);
        strcat(new_letters, &ch);
        printf("New word in upper: %s\n", new_word);
        i++;
    }
    printf("New letters in upper: %s\n", new_letters);

    printf("these are the letters length....... %ld and the letters are: %s\n", strlen(new_letters), new_letters);
    // check if the word starts with letter/letters from letter variable
    ch = letter[1];
    if(ch == (1+'0'))
    {
        if(new_word[0] != new_letters[0])
        {
            printf("[-] Word is wrong because doesn't mach input letter!\n");
            return 0;
        }
    }
    else
    {
        if(mew_word[0] != new_letters[0] || new_word[1] != new_letters[1])
        {
            printf("[-] Word is wrong because letters doesn't match!\n");
            return 0;
        }
    }

    return 1;
    // check in dictionary the word
    char buffer[100];
    bzero(buffer, 100);
    while (fgets(buffer, 100, fp))
    {
        // Remove trailing newline
        buffer[strcspn(buffer, "\n")] = 0;
        printf("WE COMPARE %s with %s\n", new_word, buffer);
        if(!strcmp(new_word, buffer))
        {
            printf("[+] Word is good!\n");
            return 1;
        }
    }

    printf("[-] Word is wrong after checking in dictionary!\n");
    return 0;
}

void sendNumberToAllPlayers(int max_number_of_players, int players_list[max_number_of_players], int number)
{
    printf("[+] Sending OK = %d to clients..\n", number);

    for (int i = 0; i < max_number_of_players; ++i)
    {
        sendNumber(number, players_list[i]);
    }
}

// function that shifts to the left all the sds and puts the first old sd from list at the end of the list
void changePlayersOrderInList(int max_number_of_players, int players_list[max_number_of_players])
{
    printf("[+] Change players list..\n");

    int first_sd = players_list[0];

    // edit the list of socket descriptors of clients
    for (int i = 1; i < max_number_of_players; ++i)
    {
        players_list[i-1] = players_list[i];
    }

    players_list[max_number_of_players - 1] = first_sd;
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

    // opening the dictionary
    FILE *fp;
    fp = fopen("Dictionary.txt", "r"); // open file

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
            // we don't need the socket descriptors for the clients
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
                int OK = 0;
                char word[100];
                char letter[3];

                while(OK == 0)
                {
                    if(max_number_of_players == 1)
                    {
                        sendNumber(999, players_list[0]); // won because last player
                        OK = 2;
                        break;
                    }

                    printf("[+] Playing first round..\n");

                    // give players the order number to know when is their turn
                    for (int i = 0; i < max_number_of_players; ++i)
                    {
                        sendNumber(i, players_list[i]);
                    }

                    // read the letter of the first player
                    readLetter(letter, players_list[0]);

                    // send the letter to the other players
                    for (int i = 1; i < max_number_of_players; ++i)
                    {
                        sendLetter(letter, players_list[i]);
                    }

                    // read the word of the first player
                    readWord(word, players_list[0]);

                    // send the word to the other players
                    for (int i = 1; i < max_number_of_players; ++i)
                    {
                        sendWord(word, players_list[i]);
                    }

                    char ch[3];
                    strcat(ch, &letter[0]);
                    strcat(ch, "1");

                    // if the word is good, announce players
                    if(wordIsGood(word, ch, fp))
                    {
                        sendNumberToAllPlayers(max_number_of_players, players_list, 1);

                        changePlayersOrderInList(max_number_of_players, players_list);
                        OK = 1;
                    }
                    else // if the player chose a wrong word
                    {
                        sendNumberToAllPlayers(max_number_of_players, players_list, 0);

                        close(players_list[0]);

                        // edit the list of socket descriptors of clients
                        for (int i = 1; i < max_number_of_players; ++i)
                        {
                            players_list[i-1] = players_list[i];
                        }
                        max_number_of_players--;
                    }
                }

                if(OK == 2)
                {
                    break;
                }

                while(OK == 1)
                {
                    getLastTwoLetters(word, letter);

                    printf("[+] Playing/Continue game after first round.\n");

                    // give players the order number to know when is their turn
                    for (int i = 0; i < max_number_of_players; ++i)
                    {
                        sendNumber(i, players_list[i]);
                    }

                    // read the word of the first player
                    readWord(word, players_list[0]);

                    // send the word to the other players
                    for (int i = 1; i < max_number_of_players; ++i)
                    {
                        sendWord(word, players_list[i]);
                    }

                    // if the word is good, announce players
                    if(wordIsGood(word, letter, fp))
                    {
                        sendNumberToAllPlayers(max_number_of_players, players_list, 1);

                        changePlayersOrderInList(max_number_of_players, players_list);
                    }
                    else // if the player chose a wrong word
                    {
                        sendNumberToAllPlayers(max_number_of_players, players_list, 0);

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

            fclose(fp); // closing the dictionary
            closePlayersList(max_number_of_players, players_list); // closing the remaining sd

            exit(0);
        }
        bzero(players_list, max_number_of_players);
    }
}