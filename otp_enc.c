/*
Name: Daniel Maynard
Program: Encode Client
Description: This will send a file with a specific port number. If the correct
daemon (otp_enc_d) is is under the same port number, it will encrypt the data
and send it back to the client. This program will check if there are any 
invalid characters and if the key is larger enough for the encode to take
place. It will then send the client verification, file and key to the server.
If a client verification of false is not received, it will recv in a loop until
a new line is found indicating the end of the file was found.

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


#define BUFFSIZE 200000
char fileName[BUFFSIZE];
char fileContent[BUFFSIZE];
char key[BUFFSIZE];
char keyContent[BUFFSIZE];
char enc[10];
char final[BUFFSIZE];
int retErr = 0;

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues





//reference: read the file data https://www.tutorialspoint.com/cprogramming/c_file_io.htm
void readFile(char* fn, char* fdesc)
{
    FILE *fp;
    char buff[BUFFSIZE-1];
    //clear 
    memset(buff, '\0', sizeof(buff));
    fp = fopen(fn, "r");

    fgets(buff, BUFFSIZE-1, (FILE*)fp);

    fclose(fp);
    //copy buffer into fileContent containing the plaintext
    strcpy(fdesc, buff);
  


   
    
}

void checkCharacters(char* fn)
{
    int i;
    int p;
    int check = 0;
    char randomletter = 'A';
    for (i = 0; i < strlen(fn); i++)
    {
     //verify if the ASCII value of the characters is between 65-90 and 32.
     //ignore spaces and new lines
        if ( ((int)fn[i] != 10 && (int)fn[i] != 32) && ((int)fn[i] > 90 || (int)fn[i] < 65))
        {
            check = 1;
        }
        
    }
    //if non valid character found set retErr to 1
    if (check == 1)
    {
        retErr = 1;
    }
}




int main(int argc, char *argv[])
{
int socketFD, portNumber, charsWritten, charsRead;
struct sockaddr_in serverAddress;
struct hostent* serverHostInfo;
//holds the plaintext
char buffer[BUFFSIZE];
//holds the key
char buffer2[BUFFSIZE];
if (argc < 4) 
{
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); 
    
} // Check usage & args

//arg 1 is the file
strcpy(fileName, argv[1]);
//arg2 is the key
strcpy(key, argv[2]);
//read the file contents in the filename
readFile(fileName, fileContent);
checkCharacters(fileContent);
//close out of client if wrong character found
if (retErr == 1)
{
    fprintf(stderr, "%s", "otp_enc error: input contains bad characters.\n");
    exit(1);
}
//read key file
readFile(key, keyContent);

//verify if key has enough characters
if (strlen(keyContent) < strlen(fileContent))
{
    fprintf(stderr, "%s", "Error: key ‘myshortkey’ is too short.\n");
    exit(1);
}

// Set up the server address struct
memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
serverAddress.sin_family = AF_INET; // Create a network-capable socket
serverAddress.sin_port = htons(portNumber); // Store the port number
char *localhost = "localhost";
serverHostInfo = gethostbyname(localhost); // Convert the machine name into a special form of address
if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);

// Set up the socket
socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
if (socketFD < 0) error("CLIENT: ERROR opening socket");
// Connect to server
if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
error("CLIENT: ERROR connecting");

strcpy(buffer,argv[1]);

//this tells the server its an from the encode client
memset(enc, '\0', sizeof(enc)); // Clear out the buffer array
strcpy(enc, "enc\n");
charsWritten = send(socketFD, enc, strlen(enc), 0); // Write to the server

// Send message to server
charsWritten = send(socketFD, fileContent, strlen(fileContent), 0); // Write to the server

//send key content to server
charsWritten = send(socketFD, keyContent, strlen(keyContent), 0); // Write to the server

// Get return message from server
memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

int numChars = 0;
int endLine = 0;
memset(final, '\0', sizeof(final)); // Clear out the buffer again for reuse  
int i;

//search for the '\n' character in the text that is received from the server
while (endLine == 0)
{
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse    
    charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); // Read data from the socket, leaving \0 at end
    strcat(final, buffer);
    for (i = 0; i < strlen(final); i++)
    {
        if (final[i] == '\n')
        {
            endLine = 1;
            break;    
        }
    }
    
    
}


if (charsRead < 0) error("CLIENT: ERROR reading from socket");

if (strcmp(final, "false\n") == 0)
{
    
    char errString[100] = "Error: could not contact otp_enc_d on port ";
    strcat(errString, argv[3]);
    strcat(errString, "\n");
    fprintf(stderr, errString);
    exit(1);
}

//print out final cyphertext
printf("%s", final);

close(socketFD); // Close the socket
return 0;
}