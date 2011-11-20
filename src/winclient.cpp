//cl winclient.cpp -link wsock32.lib
/* tcpclient.c *//*

#include "stdafx.h"
*/
#include <winsock2.h>
#include <Ws2tcpip.h>

/*
#include <types.h>
#include <in.h>
#include <netdb.h>
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <unistd.h>
//#include <errno.h>


int main(int argc, char* argv[])
{
    //----------------------
    // Declare and initialize variables.
    WSADATA wsaData;
    int iResult;

    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

        int sock, bytes_recieved;  
        char send_data[1024],recv_data[1024];
        struct hostent *host;
        struct sockaddr_in server_addr;  

        if (argc < 2)
        {
            printf("usage: %s <ip address>", argv[0]);
            return 1;
        }

        printf("%s\n", argv[1]);

        host = gethostbyname(argv[1]);
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket");
            exit(1);
        }
        
        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(5000);   
        server_addr.sin_addr = *((struct in_addr *)host->h_addr);
        ZeroMemory(&(server_addr.sin_zero),8);

        if (connect(sock, (struct sockaddr *)&server_addr,
                    sizeof(struct sockaddr)) == -1) 
        {
            perror("Connect");
            exit(1);
        }

        while(1)
        {
        
          bytes_recieved=recv(sock,recv_data,1024,0);
          recv_data[bytes_recieved] = '\0';
 
          if (strcmp(recv_data , "q") == 0 || strcmp(recv_data , "Q") == 0)
          {
            closesocket(sock);
           
           break;
          }

          else
           printf("\nRecieved data = %s " , recv_data);
           
           printf("\nSEND (q or Q to quit) : ");
           gets(send_data);
           
          if (strcmp(send_data , "q") != 0 && strcmp(send_data , "Q") != 0)
           send(sock,send_data,strlen(send_data), 0); 

          else
          {
           send(sock,send_data,strlen(send_data), 0);   
           closesocket(sock);
           
           break;
          }
        
        }
        WSACleanup();
return 0;
}
