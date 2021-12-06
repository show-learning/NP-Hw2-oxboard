#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#define MAX 300
#define PORT 8080
#define SA struct sockaddr

int clients[10]={0};
int fdmax = 10;
char* account[10];



void ListAllUser(int sockfd){
	char temp[MAX];
	char buff[MAX];
	bzero(buff, MAX);
	strcpy(buff,"Online user:\n");
	for(int i =0;i < fdmax; i++){
		if(clients[i] != 0){
			sprintf(temp,"account: %s  socket number: %d\n",account[clients[i]],clients[i]);
			strcat(buff,temp);
		}
	}
	send(sockfd,buff,sizeof(buff),0);
	//sprintf(temp,"Choose your opponent :\n");
	//send(sockfd,temp,sizeof(temp),0);
}

int CheckPwd(char* content){
	FILE *fp;
	char temp[40];
	fp = fopen("password.txt","r");
	while (fscanf(fp, "%s", temp) != EOF) {
        if(strcmp(content,temp) == 0){
			return 1;
		}
    }
	return 0;
}

int Login(int sockfd){
	char buff[MAX];
	char temp[MAX];
	
	while(1){
		bzero(buff, MAX);
		send(sockfd ,"Account",strlen("Account"),0);
		recv(sockfd,buff,sizeof(buff),0);
		printf("Account=%s\n",buff);
		strcpy(account[sockfd], buff);
		account[sockfd][strlen(buff)]='\0';
		strcpy(temp, buff);
		temp[strlen(buff)] = ':';
		send(sockfd ,"Password:",strlen("Password"),0);
		bzero(buff, MAX);
		recv(sockfd,buff,sizeof(buff),0);
		strcat(temp,buff);
		temp[strlen(temp)] = '\0';
		printf("%s\n",temp);
		//account:password
		if(CheckPwd(temp) == 1){
			send(sockfd ,"Login Successful",strlen("Login Successful"),0);
			return 1;
		}
		else{
			send(sockfd ,"Login fail",strlen("Login fail"),0);

			return 0;
		}
	}
}

// Function designed for first connect client and server.
void func(void* p)
{
	int sockfd = *(int*)p;
	printf("pthread = %d\n",sockfd);
	char buff[MAX];
	int n;
	while(1){
		//Login
		if(Login(sockfd) == 1){
			printf("Account : %s login\n",account[sockfd]);
			break;
		}
		else{
			printf("Login fail\n");
			pthread_kill(sockfd,SIGALRM);
		}
	}
	
	// infinite loop for chat
	while (1) {
		bzero(buff, MAX);
		// read the message from client and copy it in buffer
		if (recv(sockfd,buff,sizeof(buff),0) <= 0) {
			//Logout
			int i;
			for (i = 0;i < fdmax;i++){
				if (sockfd == clients[i]){
					clients[i] = 0; //這個client不存在了
					break;
				}
			}
			printf("Client %d Exit...\n",sockfd);
			pthread_exit((void*)i);
		}
		// print buffer which contains the client contents
		printf("From client %d: %s\n",sockfd, buff);
		if(strncmp("exit", buff, 4) == 0){
			for(int i = 0; i < fdmax; i++){
				if(clients[i]!= 0)
				{
					sprintf(buff,"[%s] Logout!\n",account[sockfd]);
					send(clients[i],buff,sizeof(buff),0);
				}
			}	
		}
		else if(strncmp("ls", buff, 2) == 0) {
			ListAllUser(sockfd);
			bzero(buff, MAX);
		}
		else if(buff[0] == '@'){
			//choose opponent
			char *ptr = buff;
			ptr++;
			int oppofd = atoi(ptr);
			sprintf(buff,"Invite from %s %d",account[sockfd],sockfd);
			printf("%s %d\n",buff,oppofd);
			send(oppofd,buff,sizeof(buff),0);
			send(sockfd,"Waiting opponent agree\n",strlen("Waiting opponent agree\n"),0);
		}
		else if (strncmp("Agree", buff, 5) == 0) {
			//opponent say yes
			char *ptr = buff;
			ptr = ptr + 6;
			int oppofd = atoi(ptr);
			sprintf(buff,"Agree %s %d",account[sockfd],sockfd);
			printf("%s\n",buff);
			send(oppofd,buff,sizeof(buff),0);
		}
		else if (buff[0] == '#'){
			char *ptr = buff;
			ptr++;
			int n = atoi(ptr);
			ptr = strstr(buff," ");
			ptr++;
			int oppofd = atoi(ptr);
			sprintf(buff,"#%d",n);
			printf("%s\n",buff);
			send(oppofd,buff,sizeof(buff),0);
		}
		else{
			char chat[MAX];
			sprintf(chat,"[%s]: %s",account[sockfd],buff);
			//printf("boardcast = %s",chat);
			for(int i = 0; i < fdmax; i++){
				if(clients[i]!= 0)
				{
					printf("Sending to socket %d\n%s",clients[i],chat);
					send(clients[i],chat,sizeof(chat),0);
				}
			}	
		}
		
	}
}

// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr;
	
	char temp[MAX];
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	for(int i=0; i<10; i++)
		account[i] = (char *)malloc(sizeof(char)*11);
	while (1)
	{
		struct sockaddr_in cli;
		len = sizeof(cli);
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (SA*)&cli, &len);
		if (connfd < 0) {
			printf("server accept failed...\n");
			exit(0);
		}
		printf("server accept the client...\n");
		for(int i = 0;i < fdmax;i++){
			if(clients[i] == 0){
				clients[i] = connfd;
				// login and muti connection
				printf("Number %d join connected\n",i);
				pthread_t tid;
				pthread_create(&tid,0,func,&connfd);
				//func(connfd);
				// After game close the socket
				break;
			}
			if(i == fdmax){
				strcpy(temp,"The room is full\n");
				send(connfd,temp,sizeof(temp),0);
				close(connfd);
			}
		}
	}
	for(int i=0; i<10; i++)
		free(account[i]);
}
