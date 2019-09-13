/*
Name: Daniel Maynard
Program: This is the client for decoding, its purpose is to send a cyphertext
and key to the server, which it will then decode and convert back to it's 
original text. It will check if the received data has false from the server
indicating that the wrong client is connecting to the server.
If not, then it will print to stdout the decyphered plaintext.
It will then send the client verification, file and key to the server.
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

//grab contents of the cyphertext in argv[1]
strcpy(fileName, argv[1]);
//grab contents of key
strcpy(key, argv[2]);
//read the file contents in the filename and key
readFile(fileName, fileContent);
readFile(key, keyContent);

//if key is too small exit
if (strlen(keyContent) < strlen(fileContent))
{
    fprintf(stderr, "%s", "Key is too small.\n");
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

//this tells the server its an from the encode client
memset(enc, '\0', sizeof(enc)); // Clear out the buffer array
strcpy(enc, "dec\n");
//send the client confirmation to server
charsWritten = send(socketFD, enc, strlen(enc), 0); // Write to the server

// Send cyphertext to server
charsWritten = send(socketFD, fileContent, strlen(fileContent), 0); // Write to the server

//send key content to server
charsWritten = send(socketFD, keyContent, strlen(keyContent), 0); // Write to the server

// Get return message from server
memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse

int numChars = 0;
int endLine = 0;
memset(final, '\0', sizeof(final)); // Clear out the buffer again for reuse  
int i;

//recv until we find a \n
while (endLine == 0)
{
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse    
    charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0); // Read data from the socket, leaving \0 at end
    //concatenate string into final variable until \n is found
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

//if final = false, then we are sending the wrong client to the wrong server
if (strcmp(final, "false\n") == 0)
{
    char errString[100] = "Error: could not contact otp_dec_d on port ";
    strcat(errString, argv[3]);
    strcat(errString, "\n");
    fprintf(stderr, errString);
    exit(1);
}

if (charsRead < 0) error("CLIENT: ERROR reading from socket");

//print out decyphered text
printf("%s", final);

close(socketFD); // Close the socket
return 0;
}