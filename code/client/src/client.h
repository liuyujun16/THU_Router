#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>


void get_info(char* str,int num){
    char* temp;
    temp = (char*)malloc(sizeof(char) * strlen(str));
    for(int i = 0; i < strlen(str) - num; i++){
        temp[i] = str[i + num];
    }
    strcpy(str,temp);
}

int connect_client(char *ip_address,int port_num){
	int sockfd;
	struct sockaddr_in addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	if (inet_pton(AF_INET, ip_address, &addr.sin_addr) <= 0) {
		printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		printf("Error connect(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	return sockfd;
}

int connect_server(int port_num){
	int listenfd;
	struct sockaddr_in addr;
	if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Error socket(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
	if (listenfd == -1) {
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Error bind(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}

	if (listen(listenfd, 10) == -1) {
		printf("Error listen(): %s(%d)\n", strerror(errno), errno);
		return 1;
	}
    printf("server success %d\n",listenfd);
	return listenfd;
}
void divide_port(char* str,int* port,int n){
    char temp[10];
    int cnt = 0;
    int j = 0;
    int flag = 0;
    int k = 0;
    for(int i = n; i < strlen(str); i++){
        if(str[i] != ','){
            temp[j] = str[i];
            j++;
        }
        else if(str[i] == ','){
            port[cnt] = atoi(temp);
            cnt++;
            j = 0;
            memset(temp,0,sizeof(temp));
        }
    }
    port[cnt] = atoi(temp);
}

typedef struct PORT
{
	char ip_address[20];
	int port;
}PORT;

