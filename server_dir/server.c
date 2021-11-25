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

// the port
#define PORT 2024
#define FIFO_NAME "MyTest_FIFO"

extern int errno;

// send number to socket descriptor
void sendOrder(int sd, int order)
{
    if (write(sd, &order, sizeof(int)) <= 0)
    {
      perror("[-]Error at write().\n");
    }
    else
      printf("[+]Wrote the order to the client %d.\n", order);
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

// function that see if word is in dictionary
int wordIsGood(char* word)
{
    return 1;
}

int main()
{
    int sd; // socket descriptor

    // creating the socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[-]Error at the socket() function.\n");
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
        perror("[-]Error at the bind() function.\n");
        return errno;
    }

    // listen for the client so we can accept them
    if (listen(sd, 1) == -1)
    {
        perror("[-]Error at the listen() function.\n");
        return errno;
    }

    struct sockaddr_in from; // client structure

    // getting ready the client structure
    bzero(&from, sizeof(from));

    // reading the config file for the max number of players in one game
    FILE *fp;
    fp = fopen("config_file.txt", "r");
    char file_content[255];
    fscanf(fp, "%s", file_content);
    fclose(fp);
    int max_number_of_players = atoi(file_content);
    printf("%d\n", max_number_of_players);
    

    int max = 0;
    int ok = 1;

    while (1)
    {
        int clients[255]; // client socket descriptor
        int length = sizeof(from);
        char s[300];
        int num, fd;

        printf("[+]Waiting on the port %d...\n", PORT);
        fflush(stdout);

        if(ok == 1)
        { 
            // first game from the main process
            printf("[+]1.Waiting for players to start new game...\n");
            clients[max] = accept(sd, (struct sockaddr *)&from, &length);
            ok = 0;
        }
        else
        {
            // wait for the game to get all the players and then accept more players for new game, with a FIFO

            mknod(FIFO_NAME, S_IFIFO | 0666, 0);
            fd = open(FIFO_NAME, O_RDONLY);

            printf("[main] Waiting...\n");
            if ((num = read(fd, s, 300)) == -1)
                perror("[-]Error at reading the FIFO!\n");
            else
            {
                printf("[+]o.Waiting for players to start new game...\n");
                clients[max] = accept(sd, (struct sockaddr *)&from, &length);
            }
        }

        // accepting the client
        if (clients[max] < 0)
        {
            perror("[-]Error at accepting the client.\n");
            continue;
        }
        else
        {
            int pid;
        
            // making a process for the game
            if ((pid = fork()) == -1) // error at fork
            {
                close(clients[max]);
                continue;
            }
            else if (pid > 0) // main process
            {
                close(clients[max]); // we dont need client sock

                while (waitpid(-1, NULL, WNOHANG))
                    ;
                continue;
            }
            else if (pid == 0) // game process
            {
                max = 1;

                while(max < max_number_of_players)
                {
                    printf("[+]Game in waiting for other players...\n");
                    clients[max] = accept(sd, (struct sockaddr *)&from, &length);
                    max++;
                }

                printf("[+]Game has started!\n");
                close(sd); // no more players for the game

                // let main process accept more games, with a FIFO
                fd = open(FIFO_NAME, O_WRONLY);

                if ((num = write(fd, s, strlen(s))) == -1)
                    perror("[-]Error at the writing the FIFO!\n");

                // start game with clients[]

                while(max_number_of_players != 0)
                {
                    // send the orders of the players
                    for (int i = 0; i < max_number_of_players; ++i)
                    {
                        sendOrder(clients[i], i);
                    }

                    char letter[2];

                    // read the letter of the first player
                    readLetter(clients[0], letter);

                    for (int i = 1; i < max_number_of_players; ++i)
                    {
                        sendLetter(clients[i], letter);
                    }

                    char word[100];

                    // read the word of the first player
                    readWord(clients[0], word);

                    for (int i = 1; i < max_number_of_players; ++i)
                    {
                        sendWord(clients[i], word);
                    }
/*
                    // verify word
                    if(wordIsGood(word))
                    {
                        // let the players know the game will continue
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            if (write(clients[i], 1, sizeof(int)) <= 0)
                            {
                              perror("[-]Error at write().\n");
                            }
                        close(clients[0]);
                        }

                        int copy = clients[0];

                        for (int i = 1; i < max_number_of_players; ++i)
                        {
                            clients[i-1] = clients[i];
                        }

                        clients[max_number_of_players-1] = copy;
                    }
                    else
                    {   // the first player will be kicked out
                        if (write(clients[0], 0, sizeof(int)) <= 0)
                        {
                          perror("[-]Error at write().\n");
                        }
                        close(clients[0]);

                        // edit the list of socket descriptors of clients
                        for (int i = 1; i < max_number_of_players; ++i)
                        {
                            clients[i-1] = clients[i];
                        }
                        max_number_of_players--;

                        // let the other players know the game will continue
                        for (int i = 0; i < max_number_of_players; ++i)
                        {
                            if (write(clients[i], 1, sizeof(int)) <= 0)
                            {
                              perror("[-]Error at write().\n");
                            }
                        }
                    }*/
                }
             






                printf("[+]Game done.\n");

                // close the socket descriptors of the players
                for (int i = 0; i < max_number_of_players; ++i)
                {
                    close(clients[i]);
                }

                exit(0);
            }
        }
    }
}
