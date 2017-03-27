// gcc server.c -o server
// ./server <port>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <fcntl.h>


 
/*for getting file size using stat()*/
#include <sys/stat.h>
 
/*for sendfile()*/
#include <sys/sendfile.h>

int main(int argc, char *argv[])
{
    
    int listenfd = 0, connfd = 0,k;
    struct sockaddr_in serv_addr , client; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);         // create socket 
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);              // ip address
    serv_addr.sin_port = htons(atoi(argv[1]));                      // port which in input 

    k = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));        


    k = listen(listenfd, 10); 

    

    connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);        // accept the connection of client 
    // printf("connected to the client\n");
    char buf[100],command[5],filename[20],ext[20],lscommand[20];        // defining variables 
    int size,i,filehandle;
    struct stat obj;
    int already_exits = 0;
    int overwrite_choice = 1;
    char *pos;
    // while loop start for all opreations 
    while(1)                    
    {   
        
        recv(connfd,buf,100,0);
        sscanf(buf,"%s",command);               // get command with file option 
//----------- for put command -------------------------//

        if(!strcmp(command,"put"))
        {
            int c = 0, len;
            char *f;
            
            sscanf(buf+strlen(command), "%s", filename);        // store filename in var 
            i = 1;
            // check file already exits or not 

            if( access( filename, F_OK ) != -1 )
            {
                already_exits = 1;
                send(connfd,&already_exits,sizeof(int),0);              // exits 
            } 
            else 
            {
                already_exits = 0;
                 send(connfd,&already_exits,sizeof(int),0);             // not exits 
            }
            recv(connfd,&overwrite_choice,sizeof(int),0);               // recv overwrite choice 

            // case of overwrite 
            if(already_exits==1 && overwrite_choice == 1)
            {
                filehandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);         // clear all the file data 
                recv(connfd, &size, sizeof(int), 0);
                f = malloc(size);
                recv(connfd, f, size, 0);               // recv full file data 
                c = write(filehandle, f, size);             // write data in file 
                close(filehandle);              // close file 
                send(connfd, &c, sizeof(int), 0);           // send status 

            }
            else if(already_exits == 0 && overwrite_choice == 1)            // creating the new file 
            {
                filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);   // open file 
                recv(connfd, &size, sizeof(int), 0);
                f = malloc(size);
                recv(connfd, f, size, 0);
                c = write(filehandle, f, size);
                close(filehandle);
                send(connfd, &c, sizeof(int), 0);
            }
            
            
        } // ending of put option

//------------------get option ----------------------------//
        else if(!strcmp(command,"get"))
        {
            sscanf(buf, "%s%s", filename, filename);
            stat(filename, &obj);
            filehandle = open(filename, O_RDONLY);          // open file with read only option 
            size = obj.st_size;
            if(filehandle == -1)
                 size = 0;
            send(connfd, &size, sizeof(int), 0);       // sending the size of file 
            if(size==0)
                continue;
            recv(connfd,&overwrite_choice,sizeof(int),0);   // recv over write choice 
            if(size && overwrite_choice == 1)
                sendfile(connfd, filehandle, NULL, size);       // sending the file 
      
        }// ending the get option 

//--------------------quit command----------------------------------------//
        else if(!strcmp(command, "quit"))
        {
            printf("FTP server quitting..\n");
            i = 1;
            send(connfd, &i, sizeof(int), 0);           // closing the server 
            exit(0);
        }//ending of quit option 

//---------------------mget option ---------------------------------//
        else if(!strcmp(command,"mget"))
        {
            sscanf(buf,"%s%s",ext,ext);         // get the extension 
            printf("%s\n",ext );
            strcpy(lscommand,"ls *.");
            strcat(lscommand,ext);
            strcat(lscommand,"> filelist.txt");         // run the command and store the filelist in filelist.txt 
            system(lscommand);

            char *line = NULL;
            size_t len = 0;
            ssize_t read;

            FILE *fp = fopen("filelist.txt","r");           // open the list of files 
            int num_lines = 0;
            int ch;
            while(!feof(fp))                    // count the number of files
            {
                ch = fgetc(fp);
                if(ch == '\n')
                {
                    num_lines++;
                }
            }// end of while 

            // printf("%d\n",num_lines );
            fclose(fp);                 // closing 
            fp = fopen("filelist.txt","r");                 // reopen the file list 
            send(connfd,&num_lines,sizeof(int),0);              // sending the number of files 

            while((read=getline(&line,&len,fp)) != -1)              // sending all files in while loop 
            {
                if ((pos=strchr(line, '\n')) != NULL)
                    *pos = '\0';
                strcpy(filename,line);
                send(connfd,filename,20,0);                 // sending the command 
                stat(line, &obj);
                filehandle = open(line, O_RDONLY);              // open the file only read choice 
                size = obj.st_size;
                if(filehandle == -1)
                     size = 0;
                send(connfd, &size, sizeof(int), 0);                // send the size 
                recv(connfd,&overwrite_choice,sizeof(int),0);           // recv overwrite choice 
                if(size && overwrite_choice == 1)
                    sendfile(connfd, filehandle, NULL, size);               // finally send the file 

            }// end of while loop

            fclose(fp);
            remove("filelist.txt"); 

        }// end of mget option 

    }// end of outer while loop 
}// end of main function 