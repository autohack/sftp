# sftp
File Transfer Protocol (FTP) using Client-Server socket programming


In this assignment, you require to implement two C programs, namely server and client to communicate
with each other based on TCP sockets. The goal is to implement a simple File Transfer Protocol (FTP).
Initially, server will be waiting for a TCP connection from the client. Then, client will connect to the
server using server’s TCP port already known to the client. After successful connection, the client should
be able to perform the following functionalities:

PUT: Client should transfer the file specified by the user to the server. On receiving the file,
server stores the file in its disk. If the file is already exists in the server disk, it communicates
with the client to inform it. The client should ask the user whether to overwrite the file or not and
based on the user choice the server should perform the needful action.
GET: Client should fetch the file specified by the user from the server. On receiving the file,
client stores the file in its disk. If the file is already exists in the client disk, it should ask the user
whether to overwrite the file or not and based on the user choice require to perform the needful
action.
MPUT and MGET: MPUT and MGET are quite similar to PUT and GET respectively except
they are used to fetch all the files with a particular extension (e.g. .c, .txt etc.). To perform these
functions both the client and server require to maintain the list of files they have in their disk.
Also implement the file overwriting case for these two commands as well.
Use appropriate message types to implement the aforesaid functionalities. For simplicity assume only .txt
and .c file(s) for transfer.
You should accept the IP Address and Port number from the command line (Don't use a hard-coded port
number). Prototype for command line is as follows:
Prototypes for Client and Server
Client: <executable code><Server IP Address><Server Port number>
Server: <executable code><Server Port number>
