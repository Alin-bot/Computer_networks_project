
// send number to a socket descriptor
void sendNumber(int number, int sd)
{
    if (write(sd, &number, sizeof(int)) <= 0)
    {
        perror("[-] Error at write().\n");
    }
    else
        printf("[+] Wrote the number to the socket %d.\n", number);
}

// read a number from a socket descriptor and return it
int readNumber(int sd)
{
    int number;
    fflush(stdout);
    
    printf("we ll read number from %d\n", sd);
    if (read(sd, &number, sizeof(int)) < 0)
    {
        perror("[-] Error at read().\n");
        return errno;
    }
    printf("we read\n");

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
void readLetter(int sd, char* letter)
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
    if (write(sd, word, 100) <= 0)
    {
        perror("[-] Error at write().\n");
    }
}

// read a word from a socket descriptor and return it
void readWord(int sd, char* word)
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
