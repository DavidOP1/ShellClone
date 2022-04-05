#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#define PORT 8080
#define SIZE 8196


//!---------------------------------------SEND MESSAGE TO SERVER------------------------------------------!

void sendMessage(int sock,char *output){
    int valread;
    char buffer[SIZE] = {0};
    send(sock , output, strlen(output) , 0 );
}

//!---------------------------------------CREATE CLIENT SOCKET-----------------------------------------!

int createClient(){
  int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[SIZE] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error in creating socket\n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    return sock;
}
//!-----------------------------------------CHANGING TCP MODE----------------------------------------!
int TCP_PORT(){
int sock=0;
sock= createClient();
return sock;
}
//!---------------------------------------LOCAL FUNCTION----------------------------------------!
void LOCAL(int sock, char* buffer,int tcp_mode){
  if(tcp_mode){
  sendMessage(sock,"close");
        memset(buffer,0,strlen(buffer));
        int valread = read( sock , buffer, SIZE);
        if(!strcmp(buffer,"ACK")){
        if (close(sock)<0){
          printf("Error in closing socket!\n");
        }
      }
  }
}
//!------------------------------------ECHO FUNCTION---------------------------------------------!
void ECHO(char *token,int tcp_mode, int sock,const char *s){
  char string[SIZE];
memset(string,0,strlen(string));
while (token!=NULL)
        {
           strcat(string,token); 
           if ((token=strtok(NULL,s))==NULL)    break;  
          strcat(string," ");
        }
        if(!tcp_mode) printf("%s\n",string);
        else sendMessage(sock,string);
}
//!---------------------------------------DIR FUNCTION-------------------------------------------!
void DIRE(int tcp_mode,char  * dirNames, int sock){
DIR *dirp=NULL;
memset(dirNames,0,strlen(dirNames));
    char buffer[SIZE];
    if (getcwd(buffer,SIZE)==NULL)
    {
        printf("Error in getting cwd.\n");
    }else{
         if((dirp=opendir(buffer))==NULL){  
           printf("Error in opening directory.\n");
         }else{ 
           struct dirent *dir=NULL;
             while(NULL != (dir= readdir(dirp)) )
           {
             if (!tcp_mode)
            printf(" [%s] ",dir->d_name);
            else
             strcat(dirNames,dir->d_name);
             strcat(dirNames," ");
           }
           printf("\n");
            if (tcp_mode){
             sendMessage(sock,dirNames);
             bzero(dirNames,0);
           }
           bzero(buffer,0);
           closedir(dirp);
         }
    }
}
//!-------------------------------------CD FUNCTION-------------------------------------------!
void CD(char* token, const char *s){
        char string[SIZE];
        char cwd[SIZE];
        memset(string,0,strlen(string));
    while (token!=NULL)
        {
           strcat(string,token); 
           if ((token=strtok(NULL,s))==NULL)    break;  
          strcat(string," ");
        }
    if (getcwd(cwd,SIZE)==NULL)
    {
        printf("Error in getting cwd.\n");
    }else{
      //!-------------------------------------CD dir-------------------------------------------!
      if(strcmp("..",string)){
        strcat(cwd,"/");
        strcat(cwd,string);
        //Section 7 answer: chdir() is a system call
        chdir(cwd);
        bzero(cwd,0);
        //!-------------------------------------CD .. -------------------------------------------!
      }else if(!strcmp("..",string)){
        chdir("..");
      }
    }
}
//!-------------------------------------READING FROM SRC FILE WRITING TO DEST FILE--------------------------------------------!

int dest(char *dest1,FILE *fileSrc){
  FILE * fileDest=NULL;
  int c;
  char buffer[SIZE];
  if((fileDest=fopen(dest1,"wb"))==NULL){
    printf("Failed to open %s file!\n",dest1);
    return 0;
  }
  //Section 10 answer: my implementation using fwrite()/fopen() and fclose() are all library calls.
  while(feof(fileSrc)==0){
   if((c=fread(buffer,1,sizeof(buffer),fileSrc))!=sizeof(buffer)){
    if (ferror(fileSrc)!=0)
    {
     printf("Error in reading file! \n");
     return 0;
   }
   }
   if((fwrite(buffer,1,c,fileDest))!=c){
     printf("Error in writing to file!\n");
     return 0;
   }
  }
  fclose(fileSrc);
  fclose(fileDest);
  return 0;
}

//!-------------------------------------GETTING SRC FILE POINTER---------------------------------------!
FILE * src(char *src1){
  FILE * fileSrc=NULL;
  if((fileSrc=fopen(src1,"rb"))==NULL){
    printf("Failed to open %s file!\n",src1);
    return NULL;
  }
  return fileSrc;
}
//!-----------------------------------------------COPY function----------------------------------!
void COPY(char* token, FILE* fileSrc,char* string,const char* s){
   int mone=0;
     while (token!=NULL)
        {
          if(mone==0){
           fileSrc=src(token);
           if(fileSrc==NULL) break;
          }
          else 
           dest(token,fileSrc);
           if ((token=strtok(NULL,s))==NULL)    break;  
          strcat(string," ");
          mone+=1;
        }
}
//!-----------------------------------------------System function----------------------------------!
void System(char* buffer, char* token, const char* s,int parentPid,int tcp_mode,int sock){
  int status=-1;
  char sys[SIZE];
  memset(sys,0,strlen(sys));
   while (token!=NULL)
        {
          strcat(buffer," ");
           strcat(buffer,token); 
           if ((token=strtok(NULL,s))==NULL)    break;  
        }
      //Section 8 answer: system() is a library call
      //Section 8 answer: //system(buffer); system() in a commenct as requested in section 9.
      if(!tcp_mode){
      fork();
      wait(&status);
      if(parentPid!=getpid()){
        execl("/bin/sh","sh","-c",buffer,(char*)NULL);} //This will only be executed in the child process.
      }else if(tcp_mode){
         sendMessage(sock,"system");
         int valread = read( sock , sys, SIZE);
         if(!strcmp(sys,"sys ACK"))
         {
           sendMessage(sock,buffer);
         }
      }
}
 //!--------------------------------------EXIT Function--------------------------------------------!
 void EXIT(char* string, int tcp_mode, char* buffer, int sock){
    free(string);
          if(tcp_mode) {sendMessage(sock,"close");
          int valread = read( sock , buffer, SIZE);
          if(!strcmp(buffer,"ACK")){
          if (close(sock)<0){
          printf("Error in closing socket!\n");
          }
      }
          }
          exit(EXIT_SUCCESS); 
 }
//!-----------------------------------------------Main function----------------------------------!
int main(){
    char command[SIZE],buffer[SIZE],dirNames[SIZE],test[SIZE];//Change to strtok()
    int parentPid=getpid();
    int tcp_mode=0,echoLen=0,sock=0;
    const char s[2]=" ";
    char *token,*string;
    FILE *fileSrc;
    while(1){
      printf("Yes master: ");
      //!-----------------------------------------Getting the command----------------------------------------!
      if (fgets(command,SIZE,stdin)==NULL)
      {
        printf("!Error in getting command!");
      }else if (!strcmp(command,"\n"))
      {
        continue;
      }
      else{
        //!-----------------------------------------Slicing the /n at the end----------------------------------!
        int commandLen=strlen(command);
        string=(char *)calloc(commandLen,sizeof(char));
        memcpy(string,&command[0],commandLen);
        string[commandLen-1]='\0';
        memset(buffer,0,strlen(buffer));
        //!---------------------------------------TCP PORT COMMAND------------------------------------------!
        if(!strcmp(string,"TCP PORT")){
          sock=TCP_PORT();
          tcp_mode=1;
          continue;
        }
        //!-------------------------------------------LOCAL COMMAND---------------------------------------!
        else if(!strcmp("LOCAL",string)){
          LOCAL(sock,buffer,tcp_mode);
          tcp_mode=0;
          continue;
        }
        //!----------------------------------------DIR COMMAND-------------------------------------------!
        else if(!strcmp("DIR",string)){
         DIRE(tcp_mode,dirNames,sock);
         continue;
        }
        //!--------------------------------------EXIT COMMAND--------------------------------------------!
        else if(!strcmp(string,"EXIT")){
         EXIT(string, tcp_mode, buffer, sock);
      }
      //!-----------------------------------------SLICING THE  " " CHAR'S------------------------------------------!
      token=strtok(string,s);
      strcat(buffer,token);
      token=strtok(NULL,s);
      }
      //!---------------------------------------ECHO COMMAND------------------------------------------!
      if(!strcmp(buffer,"ECHO")){
         ECHO(token,tcp_mode,sock,s);
      }
    //!----------------------------------------CD COMMAND-------------------------------------------!
    else if(!strcmp(buffer,"CD")){
     CD(token,s);
    }
    //!-----------------------------------------COPY COMMAND----------------------------------------!
    else if(!strcmp(buffer,"COPY")){
      COPY(token,fileSrc,string,s);
    }
    //!-------------------------------------DELETE command----------------------------------------------!
    else if(!strcmp(buffer,"DELETE")){
      //Section 11 answer: unlink() is a system call
      if(unlink(token)){
        printf("Unable to delete %s! \n" ,token);
      }
    }
    //!-------------------------------------System() command----------------------------------------------!
    else{
      System(buffer,token,s,parentPid,tcp_mode,sock);
    }
    //!---------------------------------------END OF COMMANDS-------------------------------------------!
  free(string);
 }
 return 0;
}
