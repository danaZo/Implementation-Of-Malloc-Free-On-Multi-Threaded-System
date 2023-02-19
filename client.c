//each client can send only one command, after one command it closes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined _WIN32

// link with Ws2_32.lib
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#else
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/socket.h>
#include <sys/types.h> 
#include <errno.h>
#include <unistd.h>
#endif

#define SERVER_PORT 3334
#define LOCAL_HOST "127.0.0.1"

int main(){
    #if defined _WIN32
    // Windows requires initialization
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d",WSAGetLastError());
            return 1;
        }
    #endif

    struct sockaddr_in serv;

    printf("Insert command for the server:\n"
           "to insert new item use: PUSH <text>\n"
           "to pop item use: POP\n"
           "to see the top item use: PEEK <text>\n"
           "to insert item to the tail use: ENQUEUE <text>\n"
           "to remove item from the tail use: DEQUEUE \n"
           "to print the satck (in the server terminal) use: print\n");    
    
    char message[1024];
    memset(message, 0, 1024);

    scanf("%[^\n]", message);

    message[strlen(message)] = '\n';

    int soc;

    soc = socket(AF_INET,SOCK_STREAM, 0);
    if (soc == -1) 
    {
        printf("Could not create socket : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }

    memset(&serv, 0, sizeof(serv));
   
    
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    serv.sin_port = htons(SERVER_PORT);

    if(connect(soc, (struct sockaddr*) &serv, sizeof(serv)) < 0)
    {
        printf("connect() failed with error code : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }

    int bytesSent = send(soc,message, strlen(message), 0);
    if (-1 == bytesSent)
    {
        printf("send() failed with error code : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }
    else if (0 == bytesSent)
    {
	printf("peer has closed the TCP connection prior to send().\n");
    }

    memset(message, 0, 1024);
    recv(soc,message,1024,0);

    printf("%s",message);
    return 0;
}