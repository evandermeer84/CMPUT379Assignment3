#include "main.h"

/*
This file generates the Transaction the T(n) and then sends them to the server
The client reads from the standard input
It can also be given the sleep command, where it goes to sleep
when the EOF is given then the client program quits
The client does not do any work between sending the transaction and waiting for the message

Log File:
Records when it sends a trans call out
Records when it receives the done signal 
Records when it goes to sleep
At the end records how many transactions were sent

Exmaple 
Using port 5002
Using server address 127.0.0.1
Host ug11.20295 // name of the machine.pid.log 

*/
FILE *fp;


int main(int argc, char *argv[]){
    // Takes in 2 arguments from the command line
    int portnum;
    char *ipaddress;
    if(argc < 3){
        printf("Error: Not enough arguments given\n");
        exit(EXIT_FAILURE);
    } else{
        //Gets a port number between 5000 and 64000 this is the same one as the server 
        portnum = atoi(argv[1]);
        // Gets an ip address
        ipaddress = argv[2];
        
    }

    //Get the name of the machine
    char hostname[MAX_MACHINE_NAME + 1];
    gethostname(hostname, MAX_MACHINE_NAME + 1);
    //Get the pid
    pid_t pid  = getpid();
    char pidStr[20];
    sprintf(pidStr, "%d", pid);
    //Create the log file 
    char * filename = NULL;
    char filenameadder[20+MAX_MACHINE_NAME+1];
    filenameadder[0] = 0;
    strcat(filenameadder, hostname);
    strcat(filenameadder, ".");
    strcat(filenameadder, pidStr);
    strcat(filenameadder, ".log");
    filename = filenameadder;  
    fp  = fopen(filename, "w"); // Create the file I am going to write to or write over the existing one
    //Writing the first part of the client file
    fprintf(fp, "Using port %d\n", portnum);
    fprintf(fp, "Using server address %s\n", ipaddress);
    fprintf(fp, "Host %s\n", hostname);

    //Open the connection 
    int sock;
	struct sockaddr_in server;
	
	
	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}

    // Set the variables 
    server.sin_addr.s_addr = inet_addr(ipaddress);
	server.sin_family = AF_INET;
	server.sin_port = htons( portnum );

    //Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

    char machinenameadder[MAX_MACHINE_NAME+ 1+ 20];
    machinenameadder[0] = 0;
    strcat(machinenameadder, hostname);
    strcat(machinenameadder, ".");
    strcat(machinenameadder, pidStr);
    if(send(sock, machinenameadder, strlen(machinenameadder), 0) < 0){
        printf("Send failed");
    }

    bool END = false;
    int transactions = 0;
    while(!END){
        //Look for the incoming input 
        int workval; // this is the value of the work whether sleep or trans
        char * line = NULL;
        size_t bufsize = 0;
        char message[20] , server_reply[20];
        //Read in  from the input
        if(getline(&line, &bufsize, stdin) == -1){
            if(feof(stdin)) {
                END = true; // We hit the end of the input
            }
        }
        if(strncmp(line, "S", 1) == 0){
            //This means that the client needs to sleep
            line++;
            workval = atoi(line);
            fprintf(fp, "Sleep %d Units\n", workval);
            Sleep(workval);

        }
        else if(strncmp(line, "T", 1) == 0){
            //This means that the client needs to send a message to the server
            line++;
            transactions++;
            workval = atoi(line);
            char workvalStr[20];
            sprintf(workvalStr, "%d", workval);
            message[0] = 0;
            //strcat(message, "T"); just sending over the value instead 
            strcat(message, workvalStr);
            //print to the FILE
            struct timeval now;
            gettimeofday(&now, NULL);
            double current_time = now.tv_sec + now.tv_usec*1e-6;
            fprintf(fp, "%.2f: (T%3d)\n", current_time, workval);

            if(send(sock, message, strlen(message), 0) < 0){
                printf("Send failed");
            }
            //Recieve a reply from the server
            if( recv(sock, server_reply, 20, 0) <0){
                printf("recv failed");
            }
            //print to the file 
            //server reply will be the transaction number
            printf("%s\n", server_reply);
            fflush(stdout);
            struct timeval here;
            gettimeofday(&here, NULL);
            double current = here.tv_sec + here.tv_usec*1e-6;
            int returnval = atoi(server_reply);
            fprintf(fp, "%.2f: (D%3d)\n", current, returnval);

        }

    }

    close(sock); // Close the connection
    fprintf(fp, "Send %d transactions", transactions);




}