
// send number to a socket descriptor
void sendNumber(int number, int sd)
{
    if (write(sd, &number, sizeof(int)) <= 0)
    {
        perror("[-] Error at write().\n");
    }
    else
        printf("[+] Wrote the number %d to the socket %d.\n", number, sd);
}

// read a number from a socket descriptor and return it
int readNumber(int sd)
{
    int number;
    fflush(stdout);
    
    if (read(sd, &number, sizeof(int)) < 0)
    {
        perror("[-] Error at read().\n");
        return errno;
    }

    return number;
}

// send a letter to a socket descriptor
void sendLetter(char *letter, int sd)
{
    if (write(sd, letter, 2) <= 0)
    {
        perror("[-] Error at write().\n");
    }
}

// read a letter from a socket descriptor and return it
void readLetter(char* letter, int sd)
{
    bzero(letter, 2);
    fflush(stdout);

    // read the letter
    if (read(sd, letter, 2) < 0)
    {
        perror("[-] Error at read().\n");
        return errno;
    }
}

// send a word to a socket descriptor
void sendWord(char *word, int sd)
{
    printf("We will send the word to the socket\n");
    if (write(sd, word, 100) <= 0)
    {
        perror("[-] Error at write().\n");
    }
    printf("Word sent\n");
}

// read a word from a socket descriptor and return it
void readWord(char* word, int sd)
{
    bzero(word, 100);
    fflush(stdout);

    // read the word
    if (read(sd, word, 100) < 0)
    {
        perror("[-] Error at read().\n");
        return errno;
    }
}
