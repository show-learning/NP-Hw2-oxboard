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

int sockfd;
char account[40];
char password[20];
int oppofd;
char opponame[20];
char oxboard[9]={"123456789"};
char player1 = 'O',player2 = 'X';
int gaming = 0,turn = 0;
void printboard()
{
	printf("%c|%c|%c\n",oxboard[0],oxboard[1],oxboard[2]);
	printf("------\n");
	printf("%c|%c|%c\n",oxboard[3],oxboard[4],oxboard[5]);
	printf("------\n");
	printf("%c|%c|%c\n",oxboard[6],oxboard[7],oxboard[8]);
}
int IsWin(char player){
	if(oxboard[0]==player&&oxboard[1]==player&&oxboard[2]==player){
		return 1;
	}
	if(oxboard[3]==player&&oxboard[4]==player&&oxboard[5]==player){
		return 1;
	}
	if(oxboard[6]==player&&oxboard[7]==player&&oxboard[8]==player){
		return 1;
	}
	if(oxboard[0]==player&&oxboard[3]==player&&oxboard[6]==player){
		return 1;
	}
	if(oxboard[1]==player&&oxboard[4]==player&&oxboard[7]==player){
		return 1;
	}
	if(oxboard[2]==player&&oxboard[5]==player&&oxboard[8]==player){
		return 1;
	}if(oxboard[0]==player&&oxboard[4]==player&&oxboard[8]==player){
		return 1;
	}
	if(oxboard[2]==player&&oxboard[4]==player&&oxboard[6]==player){
		return 1;
	}
	return 0;
}
int IsFair()
{
	int i;
	char c[2];
	for(i=0;i<9;i++)
	{
		sprintf(c,"%d\0",i+1);
		//printf("c = %s\n",c);
		if(oxboard[i]==c[0]){
			return 0;
		}
	}
	return 1;
}
void recv_message(void* p){
	//for recv
	char* ptr;
	char* qtr; 
	while (1)
	{
		char buff[MAX];
		bzero(buff,MAX);
		if (recv(sockfd,buff,sizeof(buff),0) <= 0){
			return;
		}
		else if(strncmp(buff,"Invite ",6) == 0){
			ptr = strstr(buff," ");
			ptr++;
			ptr = strstr(ptr," ");
			ptr++;
			qtr = strstr(ptr," ");
			*qtr = '\0';
			strcpy(opponame,ptr);
			qtr++;
			oppofd=atoi(qtr);
			printf("[%s] want to play a game with you,type yes if you agree\n",opponame);
		}
		else if(strncmp(buff,"Agree",5) == 0){
			//opponent agree ,set opponent information
			ptr = strstr(buff," ");
			ptr++;
			qtr = strstr(ptr," ");
			*qtr = '\0';
			strcpy(opponame,ptr);
			qtr++;
			oppofd=atoi(qtr);
			printf("Opponent %s agree the game. Ready to start!\n",opponame);
			//Game start
			strcpy(oxboard,"123456789");
			printboard();
			gaming = 1;
			player1='X';player2='O';
			if(player1 == 'O'){
				turn = 1;
			}
			else{
				turn = 0;
			}
			if(turn == 1){
				printf("Start!\n");
				
			}
			else {
				printf("Wait for %s....\n",opponame);
			}
		}
		else if(buff[0] == '#') {
			ptr = buff;
			ptr++;
			oxboard[atoi(ptr) - 1] = player2;
			printf("\n");
			printboard();
			//check win
			if(IsWin(player2)){
				printf("You Lose! Game end!\n");
				gaming = 0;
				oppofd = 0;
			}else if(IsFair())
			{
				printf("Fair! Game end!\n");
				gaming = 0;
				oppofd = 0;
			}
			else{
				turn = 1;	
				printf("It is your turn.Please enter #(1~9)\n");
			}
		}
		else{
			printf("%s\n",buff);
		}
	}
	
}

void func()
{
	//printf("pthread = %d\n",sockfd);
	char buff[MAX];

	bzero(buff, sizeof(buff));
	//Login
	while(1){
		recv(sockfd,buff,sizeof(buff),0);
		//printf("%s\n",buff);
		if ((strncmp(buff, "Account", 8)) == 0) {
			printf("Input your Account: ");
			scanf("%s",account);
			send(sockfd,account,sizeof(buff),0);
			recv(sockfd,buff,sizeof(buff),0);
			if ((strncmp(buff,"Password:", 8)) == 0) {
				printf("Input your Password: ");
				scanf("%s",password);
				send(sockfd,password,sizeof(buff),0);
				recv(sockfd,buff,sizeof(buff),0);
				if ((strncmp(buff,"Login fail", 11)) == 0) {
					printf("Login fail\n");
					return;
				}
				else if ((strncmp(buff,"Login Successful", 17)) == 0) {
					printf("Login Successful\n");
					printf("Input ls to show online user\n");
					printf("Input @(socket number) to choose opponent\n");
					printf("Input exit to Logout\n");
					break;
				}
			}
		}
	}
	pthread_t tid;
	pthread_create(&tid,0,recv_message,0);
	while (1) {
		//for send
		char input[MAX];
		bzero(input, sizeof(input));
		//printf("[%s] : ",account);
		
		scanf("%s",input);
		
		if ((strncmp(input, "exit", 4)) == 0) {
			//Logout
			printf("Client Exit...\n");
			send(sockfd, input, sizeof(input),0);
			break;
		}
		else if((strncmp(input, "ls", 2)) == 0) {
			//list all user
			send(sockfd, input, sizeof(input),0);
			
		}
		else if(input[0] == '@') {
			//choose opponent
			char* ptr = input;
			ptr++;
			printf("Choose Opponent %s...\n",ptr);
			oppofd = atoi(ptr);
			send(sockfd, input, sizeof(input),0);
		}
		else if(input[0] == '#') {
			//choose position
			if(gaming == 0){
				printf("The game is not started!\n");
			}
			else if(turn == 0){
				printf("It is not your turn!Wait for %s\n",opponame);
			}
			else{
				char* ptr = input;
				ptr++;
				int n = atoi(ptr);
				if(oxboard[n-1] =='X'|| oxboard[n-1] =='O'|| n > 9 || n < 1)
				{
					printf("Please enter another number.#(1~9)\n");
				}
				else{
					oxboard[n-1] = player1;
					printboard();
					sprintf(input,"#%d %d",n,oppofd);
					send(sockfd, input, sizeof(input),0);
					//check win
					if(IsWin(player1)){
						printf("You Win! Game end!\n");
						oppofd = 0;
						gaming = 0;
					}else if(IsFair())
					{
						printf("Fair! Game end!\n");
						oppofd = 0;
						gaming = 0;
					}
					else{						
						printf("\nWait for %s...\n",opponame);
					}
					turn = 0;
					//send to opponent (#position oppofd) 
					
					
				}
			}
		}
		else if((strncmp(input, "yes", 3)) == 0) {
			//Agree game
			if(oppofd != 0){
				printf("[%s] Agree to play\n",account);
				sprintf(input,"Agree %d",oppofd);
				send(sockfd,input,strlen(input),0);
				//start game
				strcpy(oxboard,"123456789");
				printboard();
				gaming = 1;
				player1='O';player2='X';
				if(player1=='O'){
					turn=1;
				}
				else {
					turn=0;
				}
				if(turn == 1){
					printf("Game start!Please enter #(1~9)\n");
				}
				else{
					printf("Wait for %s....\n",opponame);
				}
			}
			else{
				send(sockfd, input, sizeof(input),0);
				bzero(input, sizeof(input));
			}
		}
		else{
			send(sockfd, input, sizeof(input),0);
			bzero(input, sizeof(input));
		}
	}
}

int main()
{
	struct sockaddr_in servaddr, cli;
	// socket create and varification
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");
	
	// function for chat
	func();
	//func(sockfd);

	// close the socket
	close(sockfd);
}
