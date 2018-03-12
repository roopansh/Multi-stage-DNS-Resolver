#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080

int main(int argc, char *argv[])
{
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char * msg;
    char * server_ipaddr = argv[1];
    char * port = argv[2];
    printf("%s %s\n",argv[1],argv[2]);
    char buffer[1024] = {0};
    char type;
    char message[1023];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));

    // Convert IP addresses from text to binary form
    if(inet_pton(AF_INET, server_ipaddr, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    while(1)
    {
        memset(buffer, '\0', 1024);
        memset(msg, '\0', 1024);

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed \n");
            return -1;
        }

        printf("Packet Type:");
        scanf("%c", &type)
        printf("Message:");
        scanf("%s", message)
        strcpy(msg, type);
        strcat(msg, message);
        send(sock , msg , strlen(msg) , 0 );
        valread = read( sock , buffer, 1024);
        printf("Response Recieved : \t Type:%c\tMessage:%s\n",buffer[0], buffer+1);
        close(sock);
    }
    return 0;
}
