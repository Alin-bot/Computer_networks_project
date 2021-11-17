#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

// the port
#define PORT 2024

extern int errno;

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
        perror("setsockopt(SO_REUSEADDR) failed");
    
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

    // reading the config file
    FILE *fp;
    fp = fopen("config_file.txt", "r");
    char file_content[255];
    fscanf(fp, "%s", file_content);
    fclose(fp);
    int max_number_of_players = atoi(file_content);
    printf("%d\n", max_number_of_players);
    

    int max;
    int ok = 1;
    while (1)
    {
        int clients[255]; // client socket descriptor
        int length = sizeof(from);

        printf("[+]Waiting on the port %d...\n", PORT);
        fflush(stdout);

        if(ok == 1)
        {
            clients[1] = accept(sd, (struct sockaddr *)&from, &length);
            ok = 0;
        }
        else
        {
            // wait for game to get all the players and then accept, with a FIFO
            char s[300];
            int num, fd;

            mknod(FIFO_NAME, S_IFIFO | 0666, 0);
            fd = open(FIFO_NAME, O_RDONLY);

            if ((num = read(fd, s, 300)) == -1)
                perror("[-]Error at reading the FIFO!");
            else
                clients[1] = accept(sd, (struct sockaddr *)&from, &length);
        }

        // accepting the client
        if (clients[1] < 0)
        {
            perror("[-]Error at accepting the client.\n");
            continue;
        }
        else
        {
            max = 0;

            int pid;
        
            // making a process for the client
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
                    clients[max] = accept(sd, (struct sockaddr *)&from, &length);
                    max++;
                }

                close(sd); // no more players in the game

                // let main process accept more games, with a FIFO
                fd = open(FIFO_NAME, O_WRONLY);

                if ((num = write(fd, s, strlen(s))) == -1)
                    perror("[-]Error at the writing the FIFO!");

                // TODO: start game with clients[]

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
