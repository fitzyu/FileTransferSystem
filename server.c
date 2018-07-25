#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <stdlib.h>
#include <arpa/inet.h>
char clientIP[15];									/*文件列表*/
int sockfd;                      
int new_fd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
int sin_size,portnumber = 3333;


void handle(char cmd)
{
	char filename[100];

	int filesize = 0;
	int tmpsize = 0;
	int namesize = 0;
	int count=0;

	int fd;

	
	struct stat fstat;
	char buf[1024];

	switch(cmd)
	{	
		case 'U':
		{	
			
			/*接收文件名*/
			read(new_fd,&namesize,4);
			read(new_fd,(void *)filename,namesize);
			filename[namesize]='\0';
			
			/*创建文件*/
			if((fd=open(filename,O_RDWR|O_CREAT,0777))<0)
			{
				perror("open error:\n");	
			}
			
			/*接收文件大小*/
			read(new_fd,&filesize,4);		

			while((count=read(new_fd,(void *)buf,1024))>0)
			{
				write(fd,&buf,count);
				tmpsize += count;
				if(tmpsize==filesize)
				    break;	
			}
			
			close(fd);	
		}
		break;
		
		case 'D':
		{	
			/* 接收文件名 */
			read(new_fd,&namesize,4);
			read(new_fd,filename,namesize);
			filename[namesize]='\0';
			
			if((fd=open(filename,O_RDONLY))==-1)
			{
				perror("open: ");
				return;
			}
			
			/*发送文件长度*/
			if(stat(filename,&fstat)==-1)
				return;
			
			write(new_fd,&(fstat.st_size),4);
			
			/*发送文件内容*/
			while((count=read(fd,(void *)buf,1024))>0)
			{
				write(new_fd,&buf,count);	
			}
			
			close(fd);

		}
		break;
	}	
}


/*主函数*/
void main()
{
	int i=0;
	char cmd;
	
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket error");	
		exit(-1);
	}

	bzero(&server_addr,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(portnumber);	
	
	if(bind(sockfd,(struct sockaddr*)(&server_addr),sizeof(struct sockaddr))<0)
	{
	    perror("Bind error");
	    exit(1);		
	}
	
	if(listen(sockfd, 5)==-1)
	{
	    perror("listen error");
	    exit(1);
	}	
	
	
	while(1)
	{
		if((new_fd = accept(sockfd,(struct sockaddr *)(&client_addr),&sin_size)) == -1)
		{
			perror("accept error!");
			exit(-1);
		}
			
		//strcpy(clientIP,inet_ntoa(client_addr.sin_addr)); 				
		
		while(1)
		{
		        /*读取命令*/
			read(new_fd,&cmd,1);

			if(cmd == 'Q')
			{
                        	close(new_fd);
				break;
			}
			else
			{
				handle(cmd);
			}
		}
		close(new_fd);		
	}
	close(sockfd);
}


