
// send number to a socket descriptor
void sendNumber(int number, int sd)
{
    if (write(sd, &number, sizeof(int)) <= 0)
    {
        perror("[-] Error at write().\n");
    }

    printf("[f] Wrote the number %d to the socket %d.\n", number, sd);
}

// read a number from a socket descriptor and return it
int readNumber(int sd)
{
    printf("[f] Reading number from socket %d\n", sd);
    int number;
    fflush(stdout);

    if (read(sd, &number, sizeof(int)) < 0)
    {
        perror("[-] Error at read().\n");
        return 0;
    }

    printf("[f] Read the number %d\n", number);
    return number;
}

// send a letter to a socket descriptor
void sendLetter(char* letter, int sd)
{
    printf("[f] Sending the letter %s to the socket %d\n", letter, sd);

    if (write(sd, letter, 2) <= 0)
    {
        perror("[-] Error at write().\n");
    }

    printf("[f] Letter sent\n");
}

// read a letter from a socket descriptor and return it
void readLetter(char* letter, int sd)
{
    printf("[f] Reading letter from socket %d\n", sd);

    bzero(letter, 2);
    fflush(stdout);

    // read the letter
    if (read(sd, letter, 2) < 0)
    {
        perror("[-] Error at read().\n");
    }

    printf("[f] Read the letter %s\n", letter);
}

// send a word to a socket descriptor
void sendWord(char* word, int sd)
{
    printf("[f] Sending the word %s to the socket %d\n", word, sd);

    if (write(sd, word, 50) <= 0)
    {
        perror("[-] Error at write().\n");
    }

    printf("[f] Word sent\n");
}

// read a word from a socket descriptor and return it
void readWord(char* word, int sd)
{
    printf("[f] Reading word from socket %d\n", sd);

    bzero(word, 50);
    fflush(stdout);

    // read the word
    if (read(sd, word, 50) < 0)
    {
        perror("[-] Error at read().\n");
    }

    printf("[f] Read the word %s\n", word);
}

void getLastTwoLetters(char* word, char* letters)
{
    printf("[f] Getting the last two letters..");

    bzero(letters, 3);
    strcat(letters, &word[strlen(word) - 2]);
    strcat(letters, &word[strlen(word) - 1]); // get the last 2 letters of the last word

    printf("the last letters are: %c%c\n", letters[0], letters[1]);
}