#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

int sockfd;
int new_fd;

struct sockaddr_in server_addr;
struct sockaddr_in client_addr;

#define port 3333

void handle(char cmd){
	int namesize = 0;
	char filename[100];
	int fd;
	char buf[1024];
	int count;
	int tmpsize = 0;
	struct stat fstat;
	int filesize = 0;

	switch (cmd){
		case 'U':
			// receive file name
			read(new_fd, &namesize, 4);
			read(new_fd, (void *)filename, namesize);
			filename[namesize] = '\0';

			// create file
			fd = open(filename, O_RDWR|O_CREAT, 0777);
			// receive file length
			read(new_fd, &filesize, 4);
			// receive file content
			while((count = read(new_fd, (void *)buf, 1024)) > 0){
				write(fd, &buf, count);
				tmpsize += count;
				if (tmpsize == filesize)
					break;
			}
			close(fd);
			break;
		case 'D':
			//receive file name
			read(new_fd, &namesize, 4);
			read(new_fd, (void *)filename, namesize);
			filename[namesize] = '\0';
			// find and open  file
			fd = open(filename, O_RDONLY);
			// send file length to user
			stat(filename, &fstat);
			write(new_fd, (void *)&(fstat.st_size), 4);
			
			while((count = read(fd, (void *)buf, 1024)) > 0)
				write(new_fd, &buf, count);

			close(fd);
			break;
		default:
			break;
	}
}
int main(){
	int sin_size = sizeof(struct sockaddr);
	char cmd;

	//1.Establish a connection to the client
	//1.1 create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//1.2 bind address
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	
	bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

	//1.3 listen to the port
	listen(sockfd, 5);

	//1.4 waiting to the request
	new_fd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size);
	
	while (1){
	
	//2.respond to client request
	read(new_fd, &cmd, 1);

		if (cmd == 'Q'){
			close(new_fd);
		}
		else{
			handle(cmd);	
		}
	}

	close(sockfd);
	return 0;
}
