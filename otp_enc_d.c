/*
Name: Daniel Maynard
Program: This is the server for the encode, it will recv a client message
which will have a client verification, cyphertext and key. 
First it will allow 5 connections, and it will make sure a client is connected
and then it will fork into a child.
If the correct client is connected it will then convert file and key data to integer format
to perform arithmatic to get the correct character value. It will then
send back to the client. If the wrong client is connected it will send back
a false to the client and exit from the child.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define BUFFSIZE 200000
char fileContent[BUFFSIZE];
char keyContent[BUFFSIZE];
char plaintext[BUFFSIZE];
int childExitMethod = -5;
pid_t unfinishedProcesses[512];
int pidIncrementor = 0;

char * keyFile[3];
char encryptedFile[BUFFSIZE];
int encryptedNumber[BUFFSIZE];
char cypherText[BUFFSIZE];

//numerical value for file and key
int fileInts[BUFFSIZE];
int keyInts[BUFFSIZE];

int exitValidation = 0;



//read the plaintext file and allocate to an integer array
//if 
void readText(char* plaintext)
{
    int i;
    int p = 0;
    //dynamically allocate data for string array
    for (i = 0; i < 3; i++)
    {
        keyFile[i] = malloc(BUFFSIZE* sizeof(*keyFile[i]));
        memset(keyFile[i],'\0',BUFFSIZE* sizeof(*keyFile [i]));    
    }
    

    i = 0;
    int m = 0;
    //the plain text array holds information if its from enc or dec client
    //and the actual file. 
    //if we find a '\n' then add to the next array number
    //keyFile[0] should be the client validation
    //keyFile[1] will be the actual file.
    for (i = 0; i < strlen(plaintext); i++)
    {
        keyFile[p][m] = plaintext[i];

        m++;
        //if we found a end line move to next array element
        if (plaintext[i] == '\n')
        {
    
            //increase to next string
            p++;
            //reset character incrementer for keyfile
            m = 0;
            
        }
    }
    
    
    //check if we are receiving from the enc client
    if (strcmp(keyFile[0], "enc\n") != 0)
    {
        exitValidation = 1;
    }
    
}


//this function will do the math to convert char to integer.
void encrypt()
{
    int i;
    int m = 0;

    
    //loop through the file, last character should be a new line 
    //allocate the correct integer per character to fileInts
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
        //if a space assign t0 26
        if ((int)keyFile[1][i] == 32)
        {
            fileInts[i] = 27;
        }
        
        //if end line character, do nothing
        else if(((int)keyFile[1][i] == 10))
        {
            break;
        }
        //has to be all other types of characters
        else
        {
            fileInts[i] = (int)keyFile[1][i] - 64;
        }
        
        
    }
    
    //loop through key minus 1 for the newline character
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
        if ((int)keyFile[2][i] == 32)
        {
            keyInts[i] = 27;
        }
        
        //if end line character
        else if(((int)keyFile[2][i] == 10))
        {
            break;
        }
        else
        {
            keyInts[i] = (int)keyFile[2][i] - 64;
        }        
        
    }    

    
    // add key + file
    
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
        //get the correct modulo 26
        int calcNumber = 0;
        //add both key and file together
        calcNumber = fileInts[i] + keyInts[i];
        
        //if the combiniation is greater than 26, take the modulo 26
        if (calcNumber > 27)
        {
            calcNumber = calcNumber - 27;
        }
        
        encryptedNumber[i] = calcNumber;
    }    
    
    
    //create cypher text
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
        //if its a 26 then allocate to space
        if (encryptedNumber[i] == 27)
        {
            cypherText[i] = ' ';
            
        }
        
        //else convert to the specific char
        else
        {
            cypherText[i] = (char)(encryptedNumber[i]+64);
        }
        
    }
    //add a newline to end of the string
    cypherText[strlen(keyFile[1])] = '\n';
    strcat(cypherText, "\n");
    
}

void error(const char *msg) 
{ 
    perror(msg); exit(1); 
} // Error function used for reporting issues






int main(int argc, char *argv[])
{
int wait = 0;
int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
socklen_t sizeOfClientInfo;
char buffer[BUFFSIZE];
char keybuffer[BUFFSIZE];
struct sockaddr_in serverAddress, clientAddress;

if (argc < 2) 
{ 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); 
} // Check usage & args

// Set up the address struct for this process (the server)
memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string

serverAddress.sin_family = AF_INET; // Create a network-capable socket
serverAddress.sin_port = htons(portNumber); // Store the port number
serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

// Set up the socket
listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
if (listenSocketFD < 0) error("ERROR opening socket");

// Enable the socket to begin listening
if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
error("ERROR on binding");
listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections


pid_t childpid = 0;
int i = 0;
while(1)
{
// Accept a connection, blocking if one is not available until one connects
sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
if (establishedConnectionFD < 0) error("ERROR on accept");

pid_t spawnPid = -5;

//once we accept, fork off a child process
spawnPid = fork();

switch (spawnPid)
{
    

        case -1:
        perror("Hull Breach!");
        exit(1);
        break;     

        case 0: // Terminate the child process immediately
    
            // Get the message from the client and display it
            memset(buffer, '\0', BUFFSIZE);
            memset(keybuffer, '\0', BUFFSIZE);

            charsRead = recv(establishedConnectionFD, buffer, BUFFSIZE-1, 0); // Read the client's message from the socket

            memset(fileContent, '\0', sizeof(fileContent));
            
            //move buffer data to fileContent
            strcpy(fileContent, buffer);

            int p = 0;
            
            //process file content. which client its coming from
            //file and key data
            readText(fileContent);    
            
            //if this is the wrong client sending us info, cut the connection
            //send back false message
            if (exitValidation == 1)
            {
                char exitScript[BUFFSIZE] = "false\n";
                charsRead = send(establishedConnectionFD, exitScript, strlen(exitScript), 0); // Send success back
                exit(1);
                break;
            }
            
            //convert integers back to characters
            encrypt();
            //grab the text

            //send back ciphertext and exit from child
            charsRead = send(establishedConnectionFD, cypherText, strlen(keyFile[1]), 0); // Send success back
            if (charsRead < 0) error("ERROR writing to socket");
            exit(1);
            break;


            default:
            if (1 == 1)
            {
            waitpid(-1, &childExitMethod, 0);
            spawnPid = -5;
            break;
            }
}

}

close(establishedConnectionFD); // Close the existing socket which is connected to the client
close(listenSocketFD); // Close the listening socket
return 0;
}