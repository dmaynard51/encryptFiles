/* 
Name: Daniel Maynard
Description:Randomly print out capital letters based on the argument number used
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
int i;


int main(int argc, char *argv[])
{
    //convert to an interger
    int totalNumbers = atoi(argv[1]);
    srand(time(NULL));

    for (i = 0; i < totalNumbers; i++)
    {
        char randomletter = 'A' + (rand() % 26);
        printf("%c", randomletter);
    }
    //add a new line
    printf("\n");
    
    
    return 0;
}