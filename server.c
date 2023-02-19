/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include "stack.h"
#include "allocate.h"

#if defined _WIN32
#include<winsock2.h>   //winsock2 should be before windows
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#else
// Linux and other UNIXes
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#endif

#define PORT "3334"  // the port users will be connecting to
#define BACKLOG 10   // how many pending connections queue will hold

//Declaring the server's stack

sp server_stack;

void sendToClient(int sockid, char * message){
    int len = strlen(message);
    char str[1025] = "OUTPUT:";
    strcpy((str + 7),message);
    strcpy((str + 7 + len),"\n\0");
    if (send(sockid, str, strlen(str), 0) == -1){
        printf("send() failed with error code : %d" 
        #if defined _WIN32
                ,WSAGetLastError()
        #else
                ,errno
        #endif
                );
        #if defined _WIN32
            closesocket(sockid);
            WSACleanup();
        #else
            close(sockid);
        #endif 
    }
    #if defined _WIN32
        closesocket(sockid);
        WSACleanup();
    #else
        close(sockid);
    #endif 
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void *handle_cilent(void * arg);


int main(void)
{
    #if defined _WIN32
    // Windows requires initialization
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    #else
        signal(SIGPIPE, SIG_IGN); // on linux to prevent crash on closing socket
    #endif

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo *servinfo, *p, hints;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
   
    char s[INET6_ADDRSTRLEN];
    int rv;

    

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1)
        {
            printf("Could not create listening socket : %d" 
            #if defined _WIN32
                    ,WSAGetLastError()
            #else
                    ,errno
            #endif
                    );
            continue;
        }
        int enableReuse = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
            #if defined _WIN32
		        (const char*)
            #endif
		        &enableReuse, 
		        sizeof(int)) < 0)
        {
        printf("setsockopt() failed with error code : %d" , 
        #if defined _WIN32
		    WSAGetLastError()
        #else
		    errno
        #endif
		    );
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            printf("Bind failed with error code : %d" , 
            #if defined _WIN32
                WSAGetLastError()
            #else
                errno
            #endif
                );

            #if defined _WIN32
                closesocket(sockfd);
                WSACleanup();
            #else
                close(sockfd);
            #endif 
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        printf("listen() failed with error code : %d"
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
        #if defined _WIN32
            closesocket(sockfd);
            WSACleanup();
        #else
            close(sockfd);
        #endif    
        
        exit(1);
    }

    printf("server: waiting for connections...\n");
    pthread_t tid;

    server_stack = create_stack(2); //initiate the stack.
    
    while(1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            printf("listen failed with error code : %d"
            #if defined _WIN32
		        ,WSAGetLastError()
            #else
		        ,errno
            #endif
			);
            #if defined _WIN32
                closesocket(sockfd);
                WSACleanup();
            #else
                close(sockfd);
            #endif
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if( pthread_create(&tid, NULL, handle_cilent, &new_fd) != 0){
            perror("Thread creating failed");
        }
    }

    return 0;
}


void *handle_cilent(void * arg){
        
        int client_soc = *(int*) arg; //get scok id
        
        char message[1024], *answer;
        memset(message, 0, 1024);
        char * CR_index;

        
        if(recv(client_soc, message, 1024, 0) < 0){
            printf("recv() failed with error code : %d" 
            #if defined _WIN32
			    ,WSAGetLastError()
            #else
			    ,errno
            #endif
			    );
            #if defined _WIN32
                closesocket(client_soc);
                WSACleanup();
            #else
                close(client_soc);
            #endif    
            
        }
        
        //replace CRLF with LF in case the os use it
        CR_index = strchr(message, '\r');
        if(CR_index){
            *CR_index = '\n';
            *(CR_index +1) = 0; //replace the old LF with null
        }

        printf("%s", message);

        // execute the  command
        if(strncmp("POP", message, 3) == 0){

            answer = pop(server_stack, FALSE);
            if(!answer){ // stack is empty
                
                sendToClient(client_soc,"Stack is empty.\n");
                return arg;
                }
            
            sendToClient(client_soc,answer);
            
        }
        if(strncmp("DEQUEUE", message, 7) == 0){

            answer = deque(server_stack);
            if(!answer){ // stack is empty
                
                sendToClient(client_soc,"Stack is empty.\n");
                
                return arg;
                }
            
            sendToClient(client_soc,answer);
            
        }
        else if (strncmp("PUSH ", message, 5) == 0){
            char * tok;
            tok = strtok(message, "\n");
            tok += 5;
            push(server_stack, tok, 0);
        }
        else if(strncmp("ENQUEUE ", message , 8) == 0){
            char * tok;
            tok = strtok(message, "\n");
            tok += 8;
            enque(server_stack, tok);
        }

        if(strncmp("TOP", message, 3) == 0){

            answer = peek(server_stack);
            if(!answer){ // stack is empty
                
                sendToClient(client_soc,"Stack is empty.\n");
                return arg;
                }
            
            sendToClient(client_soc,answer);
         
        }
        else if (strcmp("print\n", message) == 0){
            print_stack(server_stack);
        }
        #if defined _WIN32
            closesocket(client_soc);
            WSACleanup();
        #else
            close(client_soc);
        #endif
           
        return NULL;

}