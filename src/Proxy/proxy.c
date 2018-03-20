// Server side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

struct queue
{
    char ip_address[1024];
    char domain_name[1024];
    struct queue * next;
};

struct queue * cache_head;

int find_entry_in_cache(char *x) {
    int type = x[0]-'0';
    char message[1023];
    strcpy(message, x+1);
    struct queue *ptr = cache_head;
    while (ptr != NULL){
        if (type == 2 && strcmp(message, ptr->ip_address)==0) {
            return 1;
        } else if (type == 1 && strcmp(message, ptr->domain_name)==0) {
            return 1;
        }
        ptr=ptr->next;
    }
    return 0;
}

void get_entry_in_cache(char *x, char *y) {
    int type = x[0]-'0';
    char message[1023];
    strcpy(message, x+1);

    struct queue *ptr = cache_head;
    while (ptr != NULL){
        if (type == 2 && strcmp(message, ptr->ip_address)==0) {
            strcpy(y, "3");
            strcat(y+1, ptr->domain_name);
            return;
        } else if (type == 1 && strcmp(message, ptr->domain_name)==0) {
            strcpy(y, "3");
            strcat(y+1, ptr->ip_address);
            return;
        }
        ptr=ptr->next;
    }
    return;
}

void empty(char * x)
{
    int i;
    for(i=0;i<1024;i++)x[i]='\0';
}


int main(int argc, char const *argv[])
{

    switch (argc ){
        case 2:
            break;
        default:
            fprintf(stderr, "%s\n", "Usage : ./proxy <port>");
            exit(1);
            break;
    }

    //declaring cache

    char * PORT = argv[1];

    cache_head=NULL;

    int server_fd, new_socket, valread;
    struct sockaddr_in address;int dns_sock=0;

    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char returnbuffer[1024] = {0};

    int i;
    for(i=0;i<1024;i++)buffer[i]='\0';
    for(i=0;i<1024;i++)returnbuffer[i]='\0';
    /*connecting with dns*/

    struct sockaddr_in dns_serv_addr;
    if ((dns_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&dns_serv_addr, '0', sizeof(dns_serv_addr));

    dns_serv_addr.sin_family = AF_INET;
    dns_serv_addr.sin_port = htons(8080);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "172.16.114.163", &dns_serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(dns_sock, (struct sockaddr *)&dns_serv_addr, sizeof(dns_serv_addr)) < 0)
    {
        printf("\nConnection with DNS Failed \n");
        return -1;
    }

    close(dns_sock);

    /*-------------------------------------*/


    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

 	memset(&address, '0', sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( atoi(PORT) );

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        valread = read( new_socket , buffer, 1024);
        printf("Request :\tType-%c,\tMessage-%s\n",buffer[0], buffer+1 );

        if(buffer[0]=='1'||buffer[0]=='2')
        {
            if(find_entry_in_cache(buffer))
            {
                get_entry_in_cache(buffer,returnbuffer);
                empty(buffer);
                send(new_socket , returnbuffer , strlen(returnbuffer) , 0 );
                empty(returnbuffer);
                close(new_socket);
            	printf("CACHE\n");
            }
            else
            {
                // printf("%s\n",buffer);
                int var=buffer[0]=='1'?1:0;
                int i;
                struct queue * temp;
                temp = (struct queue *)malloc(sizeof(struct queue));
                for(i=0;i<1024;i++)temp->domain_name[i]='\0';
                for(i=0;i<1024;i++)temp->ip_address[i]='\0';
                if(buffer[0]=='1')for(i=1;i<strlen(buffer);i++)temp->domain_name[i-1]=buffer[i];
                else    for(i=1;i<strlen(buffer);i++)temp->ip_address[i-1]=buffer[i];

                 if ((dns_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    printf("\n Socket creation error \n");
                    return -1;
                }

                if (connect(dns_sock, (struct sockaddr *)&dns_serv_addr, sizeof(dns_serv_addr)) < 0)
                {
                    printf("\nConnection with DNS Failed \n");
                    return -1;
                }

                send(dns_sock , buffer , strlen(buffer) , 0 );
                empty(buffer);
                valread = read( dns_sock , buffer, 1024);
                close(dns_sock);
                // printf("%s\n",buffer);

                if(buffer[0]=='3')
                {
                    if(var) for(i=1;i<strlen(buffer);i++)temp->ip_address[i-1]=buffer[i];
                    else for(i=1;i<strlen(buffer);i++)temp->domain_name[i-1]=buffer[i];
                    int size=0;
                    temp->next=cache_head;
                    cache_head=temp;

                    struct queue * current=cache_head;
                    while(current->next!=NULL){current=current->next;size++;}
                    if(size>=3)
                    {
                    free(current);
                    current=cache_head;
                    current->next->next->next=NULL;
                    }
                    printf("UPDATE CACHE\n");
                }
                else free(temp);

                send(new_socket ,buffer , strlen(buffer) , 0 );
                close(new_socket);
                printf("Response :\tType-%c,\tMessage-%s\n", buffer[0], buffer+1);
                empty(buffer);
            }
        }
        else
        {
            empty(buffer);
            strcpy(buffer,"4ErrorMessage");
            send(new_socket ,buffer , strlen(buffer) , 0 );
            close(new_socket);
            empty(buffer);
        }

    }
    return 0;
}
