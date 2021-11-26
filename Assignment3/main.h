#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <netinet/in.h>

#define MAX_MACHINE_NAME 100
#define TRUE   1 
#define FALSE  0 


//Function definitions
void Sleep( int n );
void Trans( int n );



