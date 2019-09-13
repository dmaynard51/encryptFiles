/*
Name: Daniel Maynard
Program: This is the server for the decode, it will recv a client message
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


int convertLetter(char letter)
{
    
}

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
    //read the whole file
    for (i = 0; i < strlen(plaintext); i++)
    {
        keyFile[p][m] = plaintext[i];
        //printf("%c", keyFile[p][m]);
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
    if (strcmp(keyFile[0], "dec\n") != 0)
    {
        exitValidation = 1;
    }
    
}

void encrypt()
{
    int i;
    int m = 0;
    
    //loop through the file, last character should be a new line 
    //allocate integers for each character in the file
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
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
            //printf("%i ", (int)keyFile[1][i] - 64);
            fileInts[i] = (int)keyFile[1][i] - 64;
        }
        
        
    }
    
    //loop through key minus 1 for the newline character
    //allocate integers for each character
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
    
    //perform arithmetic for the file - key
    for (i = 0; i < strlen(keyFile[1])-1; i++)
    {
        //get the correct modulo 26
        int calcNumber = 0;
        //add both key and file together
        calcNumber = fileInts[i] - keyInts[i];
        
        //if the combiniation is greater than 26, take the modulo 26
        if (calcNumber <= 0)
        {
            calcNumber = calcNumber + 27;
        }
        
        //final integer value for the character
        encryptedNumber[i] = calcNumber;
        
    }    
    
    
    //convert integer values back to character
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
    //add on a \n at the end
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
//return to parent


    



// Accept a connection, blocking if one is not available until one connects
sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
if (establishedConnectionFD < 0) error("ERROR on accept");
    
pid_t spawnPid = fork();

switch(spawnPid)
{
case -1:
        perror("Hull Breach!");
        exit(1);
        break;     

case 0: // Terminate the child process immediately
    
        
            // Get the message from the client and display it
            memset(buffer, '\0', BUFFSIZE);
            memset(keybuffer, '\0', BUFFSIZE);

            //recv from client
            charsRead = recv(establishedConnectionFD, buffer, BUFFSIZE-1, 0); // Read the client's message from the socket
            memset(fileContent, '\0', sizeof(fileContent));
            strcpy(fileContent, buffer);

            int p = 0;
            
            //allocate file and key and convert them to integers
            readText(fileContent);
            
            //if what we receive back is false, then its from the wrong client
            if (exitValidation == 1)
            {
                char exitScript[BUFFSIZE] = "false\n";
                charsRead = send(establishedConnectionFD, exitScript, strlen(exitScript), 0); // Send success back
                exit(1);
                break;
            }
            //convert integers back to characters
            encrypt();
            
            //send back ciphertext
            charsRead = send(establishedConnectionFD, cypherText, strlen(keyFile[1]), 0); // Send success back
            if (charsRead < 0) error("ERROR writing to socket");

            exit(1);
            
            //exit out of child
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