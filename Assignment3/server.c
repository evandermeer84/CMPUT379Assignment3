#include "main.h"
// Ten concurrent clients 

/*
Recieves the t(n) from the clients and then executes the transactions 
maintians a transaction counter that goes up each time a transaction is given
after a transaction is completed the server sends a message to the client D<#> starting at 1 letting the client know that it is done
Needs to keep track of the last transaction that was completed, if after 30 seconds after that time, nothing happends the program quits

Log File:
Starts with the port 
named machine.pid.log
Finishes with summary statistics in real time, not CPU, from the last transaction completed 
*/

FILE *fp;

int main(int argc, char *argv[]){

    //Global Variables
    int portnum;
    //struct sockaddr_in server , client;


    if(argc < 2){
        printf("Error: Not enough arguments given\n");
        exit(EXIT_FAILURE);
    } else{
        portnum = atoi(argv[1]); //port should be between 5000 and 64000 - verify this
    }

    //Create the output file
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
    //Open the file
    fp  = fopen(filename, "w"); // Create the file I am going to write to or write over the existing one 
    fprintf(fp, "Using port %d\n", portnum);
    
    

    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[10] , 
          max_clients = 10 , activity, i , valread , sd;  
    int max_sd;  
    int transactions[max_clients];
    char transaction_names[max_clients][MAX_MACHINE_NAME+1];
    struct sockaddr_in address;  
         
    
         
    //set of socket descriptors 
    fd_set readfds;  
         
    //the total number of transactions
    int transactions_num = 0;
     
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0; 
        transactions[i] = 0; // This makes all of the intial transactions also 0; 
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections , 
    //this is just a good habit, it will work without this 
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
          sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( portnum );  
         
    //bind the socket to localhost portnum given
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", portnum);  
         
    //try to specify maximum of 10 pending connections for the master socket 
    if (listen(master_socket, 10) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  


    bool END = false; 
    struct timeval lastone;
    struct timeval begin;
    while(!END)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        
        struct timeval tv = {30, 0};
        gettimeofday(&lastone, NULL);
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &tv); 
        struct timeval here;
        gettimeofday(&here, NULL);
        double time_elasped = here.tv_sec - lastone.tv_sec;
        if(time_elasped >= 30){
            END = TRUE;
        }
       
        if ((activity < 0))  
        {  
            printf("select error");  
        }  
             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket, 
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket , inet_ntoa(address.sin_addr) , ntohs
                  (address.sin_port));  
            
                 
            puts("Welcome message sent successfully");  
                 
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    transactions[i] = 0;
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }  
        }  
             
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];  
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                char buffer[121] = {'\0'};  //data buffer of 1K 
                if ((valread = read( sd , buffer, 121)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , \
                        (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , 
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;  
                }  
                     
                //Echo back the message that came in 
                else 
                {  
                    if(transactions_num == 0){
                        gettimeofday(&begin, NULL);
                    }
                    if(transactions[i] == 0){
                        transaction_names[i][0] = 0;
                        strcat(transaction_names[i], buffer);
                        transactions[i]++;
                    } else if(transactions[i] > 0){
                        transactions_num++;
                        printf("%s\n", buffer);
                        int workval = atoi(buffer);
                        struct timeval now;
                        gettimeofday(&now, NULL);
                        double current_time = now.tv_sec + now.tv_usec*1e-6;
                        fprintf(fp, "%.2f: # %2d (T%3d) from %s\n", current_time, transactions_num, workval, transaction_names[i]);
                        fflush(fp);
                        Trans(workval);
                        //set the string terminating NULL byte on the end 
                        //of the data read 
                        char transsend[12];
                        sprintf(transsend, "%d", transactions_num);  
                        send(sd , transsend , strlen(transsend) , 0 );  
                        //Print that it was Done
                        gettimeofday(&now, NULL);
                        current_time = now.tv_sec + now.tv_usec*1e-6;
                        fprintf(fp, "%.2f: # %2d (Done) from %s\n", current_time, transactions_num, transaction_names[i]);
                        fflush(fp);
                        transactions[i]++;
                    }
                    
                    
                    
                }  
            }  
        }  
    }  
    //Print the summary of the connections
    fprintf(fp, "SUMMARY\n");
    for(int i = 0; i < max_clients; i++){
        if (transactions[i] > 0){
            fprintf(fp, "%4d transactions from %s\n", transactions[i], transaction_names[i]);
        }
    }
    double seconds_taken = lastone.tv_sec - begin.tv_sec;
    double mseconds_taken = lastone.tv_usec - begin.tv_usec;
    double time_taken = seconds_taken + mseconds_taken*1e-6;
    double average = (double) transactions_num/ time_taken;
    fprintf(fp, "%.2f transactions/sec   (%d/%.2f)\n", average, transactions_num, time_taken);

}