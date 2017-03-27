// gcc client.c -o client
// ./client <ip_add> <port>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/stat.h>
 
#include <sys/sendfile.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> \n",argv[0]);		// checking argument 
        return 1;
    } 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");			// socket create
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 			//assigning  server address

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2])); 				// assigning port

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)			// connect to the server
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 

    int choice;
    char choice_str[1000];
    char filename[20],buf[100],ext[20],command[20];
    FILE *fp;
    int filehandle;
    struct stat obj;
    int size,status,i=1;
    char *f;
    int already_exits=0;
    int overwirte_choice = 1;
    char *pos;
    int num_lines;


    while(1)
    {
        printf("Enter a choice:\n1- put\n2- get\n 3- mput\n 4-mget\n 5-quit\n");

        fgets(choice_str,1000,stdin);
        choice_str[strlen(choice_str)-1] = '\0';
        if(strlen(choice_str)>1)
        {
        	printf("error\n");
        	continue;
        }
        choice = atoi(choice_str);
        switch(choice)
        {

        //--------put file in server----------------//
            case 1:														
                printf("enter the filename to put in server\n");
                scanf("%s",filename);							// read the file name 
                if( access( filename, F_OK ) == -1 )
	            {
	                printf(" %s does not exits in client side \n",filename );
	                break;
	            } 
                filehandle = open(filename,O_RDONLY);
                strcpy(buf,"put ");
                strcat(buf,filename);
                send(sockfd,buf,100,0);						// send put command with filename
                recv(sockfd,&already_exits,sizeof(int),0);
                if(already_exits){
                    printf("same name file already exits in server 1. overwirte 2.NO overwirte\n");	// file is already exits
                    scanf("%d",&overwirte_choice);
                }
                send(sockfd,&overwirte_choice,sizeof(int),0);			// sending overwrite choice 
                if(overwirte_choice==1)
                {
                    stat(filename, &obj);
                    size = obj.st_size;
                    send(sockfd, &size, sizeof(int), 0);
                    sendfile(sockfd, filehandle, NULL, size);				// sending file 
                    recv(sockfd, &status, sizeof(int), 0);
                    if(status)
                        printf("%s File stored successfully\n",filename);							//status 
                    else
                        printf("%s File failed to be stored to remote machine\n" , filename); 
                }
                
              break;
//----------------get file from server-------------------//
            case 2:
                printf("Enter filename to get: ");
                scanf("%s", filename);
                strcpy(buf, "get ");
                strcat(buf, filename);

                send(sockfd, buf, 100, 0);				//send the get command with file name

                recv(sockfd, &size, sizeof(int), 0);
                if(!size)
                {
                    printf("No such file on the remote directory\n\n");			//file doesn't exits
                    break;
                }

                if( access( filename, F_OK ) != -1 ) 
                {
                    already_exits = 1;
                    printf("same name file already exits in client 1. overwirte 2.NO overwirte\n");		// file already exits 
                    scanf("%d",&overwirte_choice);
                }
                send(sockfd,&overwirte_choice,sizeof(int),0);

                if(already_exits==1 && overwirte_choice == 1)
                {
                    filehandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);		// open file with all clear data 
                    f = malloc(size);
                    recv(sockfd, f, size, 0);
                    write(filehandle, f, size);
                    close(filehandle);
                }
                else if( already_exits ==0 && overwirte_choice == 1)
                {
                    filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);		// open new file 
                    f = malloc(size);
                    recv(sockfd, f, size, 0);
                    write(filehandle, f, size);
                    close(filehandle);
                }               
                break;
//------------------quit the server-------------------------//
            case 5:
                strcpy(buf, "quit");
                send(sockfd, buf, 100, 0);					// sending quit command for closing both server and client
                recv(sockfd, &status, 100, 0);
                if(status)
                {
                    printf("Server closed\nQuitting..\n");
                    exit(0);
                }
                printf("Server failed to close connection\n");		// faild to quit
                break;
//------------------mput server ----------------------------------//
            case 3:
                printf("enter the extension , you want to put in server:\n");
                scanf("%s",ext);									//  take the extionsion 
                							
                strcpy(command,"ls *.");
                strcat(command,ext);
                strcat(command," > temp.txt");						// store all file list 
                // printf("%s\n",command);
                system(command);


                char *line = NULL;                      //intilize file var
                size_t len =0;
                ssize_t read;
                FILE *fp = fopen("temp.txt","r");
                while ((read = getline(&line, &len, fp)) != -1)             // read input
                {   
                    if ((pos=strchr(line, '\n')) != NULL)
                        *pos = '\0';

                    filehandle = open(line,O_RDONLY);				// open files line wise 
                    strcpy(buf,"put ");
                    strcat(buf,line);

                    send(sockfd,buf,100,0);
                    recv(sockfd,&already_exits,sizeof(int),0);
                    if(already_exits){
                        printf("%s file already exits in server 1. overwirte 2.NO overwirte\n",line); // overwrite option for that particular file
                        scanf("%d",&overwirte_choice);
                    }
                    send(sockfd,&overwirte_choice,sizeof(int),0);			// sending overwrite choice 
                    if(overwirte_choice==1)
                    {
                        stat(line, &obj);
                        size = obj.st_size;
                        send(sockfd, &size, sizeof(int), 0);
                        sendfile(sockfd, filehandle, NULL, size);
                        recv(sockfd, &status, sizeof(int), 0);
                        if(status)											// status 
                            printf("%s stored successfully\n",line);
                        else		
                            printf("%s failed to be stored to remote machine\n",line); 
                    }
                    overwirte_choice = 1;						// re-assign overwrite choice 
                } // end of while 
                fclose(fp);			// close the file 
                remove("temp.txt");
                break;
//----------------mget server------------------------------------//
            case 4:
                printf("enter the extension , you want to get from server:\n");				
                scanf("%s",ext);									// take input the files extension 
                strcpy(buf,"mget ");
                strcat(buf,ext);
                send(sockfd,buf,100,0);									// sending buffer with choice and extension 
                recv(sockfd,&num_lines,sizeof(int),0);					// get number of files

                while(num_lines > 0)
                {


                    recv(sockfd,filename,20,0);							// recv file name 
                    recv(sockfd, &size, sizeof(int), 0);				// recv the size of file 
                    if(!size)
                    {
                        printf("No such file on the remote directory\n\n");				// error handling 
                        break;
                    }

                    if( access( filename, F_OK ) != -1 ) 				// checking if already exits or not 
                    {
                        already_exits = 1;
                        printf("%s file already exits in client 1. overwirte 2.NO overwirte\n",filename);
                        scanf("%d",&overwirte_choice);					// taking overwirte choice 
                    }
                    send(sockfd,&overwirte_choice,sizeof(int),0);			// sending overwrite choice 

                    if(already_exits==1 && overwirte_choice == 1)				// option according to the choice
                    {
                        filehandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 644);				// clear all the file
                        f = malloc(size);
                        recv(sockfd, f, size, 0);
                        write(filehandle, f, size);								// send file 
                        close(filehandle);
                    }
                    else if( already_exits ==0 && overwirte_choice == 1)
                    {
                        filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);				// open new file 
                        f = malloc(size);
                        recv(sockfd, f, size, 0);
                        write(filehandle, f, size);
                        close(filehandle);
                    }
                    overwirte_choice = 1;
                    already_exits = 0;
                    num_lines--; 
                }
                
                break;

//------------- default choice --------------------//
            default:
            	printf("choose the vaild option\n");
            	break;



        }// end of switch 

    }// end of while 

    return 0;
}// end of main