/*
To run the tests: first run the server , then run test_server
each client can send only one command, after one command it closes.
*/


#include <assert.h> // for the tests

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h> // threads functions

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


#define PORT 3334  // the port users will be connecting to
#define LOCAL_HOST "127.0.0.1"

#define BACKLOG 10   // how many pending connections queue will hold

 
int push_pop_soc, top_soc;
char message[1024];

// also for dequeue and enqueue
void Push_Pop_Soc(int client_num, char* command){

    struct sockaddr_in serv;

    push_pop_soc = socket(AF_INET, SOCK_STREAM, 0);
    if(push_pop_soc == -1)
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
    serv.sin_port = htons(PORT);


    if (connect(push_pop_soc, (struct sockaddr *) &serv, sizeof(serv)) == -1)
    {
	    printf("connect() failed with error code : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }

    printf("client %d connected to server\n", client_num);

    int bytesSent = send(push_pop_soc , command, 1024 , 0);
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

    printf("%s\n", command);

    sleep(2);

}

    
void Top_Soc(int client_num){

    struct sockaddr_in serv;

    top_soc = socket(AF_INET, SOCK_STREAM, 0);
    if(top_soc == -1)
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
    serv.sin_port = htons(PORT);

    if (connect(top_soc, (struct sockaddr *) &serv, sizeof(serv)) == -1)
    {
	   printf("connect() failed with error code : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }
    printf("client %d connected to server\n", client_num);

    printf("TOP\n");


    int bytesSent2 = send(top_soc , "TOP", 1024 , 0);
    if (-1 == bytesSent2)
    {
        printf("send() failed with error code : %d" 
        #if defined _WIN32
            ,WSAGetLastError()
        #else
            ,errno
        #endif
            );
    }
    else if (0 == bytesSent2)
    {
	printf("peer has closed the TCP connection prior to send().\n");
    }

     
    if (recv(top_soc, message, 1024, 0) == -1) {
        printf("recv() failed with error code : %d" 
            #if defined _WIN32
			    ,WSAGetLastError()
            #else
			    ,errno
            #endif
			    );
            #if defined _WIN32
                closesocket(top_soc);
                WSACleanup();
            #else
                close(top_soc);
            #endif    
    }
    
}    
       

int main()
{
    #if defined _WIN32
    // Windows requires initialization
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d",WSAGetLastError());
            return 1;
        }
    #endif

    Push_Pop_Soc(1, "PUSH 1"); // top is 1, stack is [1]
    sleep(2);

    Push_Pop_Soc(2, "PUSH 2"); //top is 2, stack is [1,2]
    sleep(2);

    Push_Pop_Soc(3, "PUSH 3"); //top is 3, stack is [1,2,3]
    sleep(2);

    Push_Pop_Soc(4, "POP"); // top is 2, stack is [1,2]
    sleep(2);

    Top_Soc(5); //should give 2
    sleep(2);
    
    /* Assert should not exit in this case since &message[0] is not NULL  */
    assert(&message[0]);
    printf("%s", &message[0]);    
    assert(strncmp(&message[0], "OUTPUT:2", 8)==0);

    Push_Pop_Soc(6, "ENQUEUE 6"); // top is 2, stack is [6,1,2]
    sleep(2);

    Push_Pop_Soc(7, "POP"); // top is 1, stack is [6,1]
    sleep(2);

    Top_Soc(8); //should give 1
    sleep(2);
    
    /* Assert should not exit in this case since &message[0] is not NULL  */
    assert(&message[0]);
    printf("%s", &message[0]);    
    assert(strncmp(&message[0], "OUTPUT:1", 8)==0);

    Push_Pop_Soc(9, "POP"); // top is 6, stack is [6]
    sleep(2);

    Top_Soc(10); // should give 6
    sleep(2);

     /* Assert should not exit in this case since &message[0] is not NULL  */
    assert(&message[0]);
    printf("%s", &message[0]);    
    assert(strncmp(&message[0], "OUTPUT:6", 8)==0);

    #if defined _WIN32
            closesocket(push_pop_soc);
            WSACleanup();
    #else
            close(push_pop_soc);
    #endif 

    #if defined _WIN32
            closesocket(top_soc);
            WSACleanup();
    #else
            close(top_soc);
    #endif  
    

    

    return(0);

}
