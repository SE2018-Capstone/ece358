/*
 * @brief: a sample tcp server program for ECE358 course project 
 * @file: uc.c
 * @author ECE358 Teaching Staff 
 * @date 2017/05/02
 * Acknowledgement: Based on code from our past TA Meng Yang  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	if(argc != 3) {
		printf("Usage: %s server-ip server-port\n", argv[0]);
		return -1;
	}

	struct sockaddr_in server;
	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	if(!inet_aton(argv[1], &(server.sin_addr))) {
		perror("invalid server-ip"); return -1;
	}
	server.sin_port = htons(atoi(argv[2]));

	int sockfd = -1;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket"); return -1;
	}

	struct sockaddr_in client;
	bzero(&client, sizeof(struct sockaddr_in));
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = 0; // Let OS choose.
	if(bind(sockfd, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) < 0) {
		perror("bind"); return -1;
	}

	socklen_t alen = sizeof(struct sockaddr_in);
	if(getsockname(sockfd, (struct sockaddr *)&client, &alen) < 0) {
		perror("getsockname"); return -1;
	}

	printf("client associated with %s %d\n",
			inet_ntoa(client.sin_addr), ntohs(client.sin_port));

	size_t buflen = 16;
	char buf[buflen];
	strcpy(buf, "hello");
	ssize_t sentlen;
	if((sentlen = sendto(sockfd, buf, strlen(buf), 0,
					(const struct sockaddr *)&server,
					sizeof(struct sockaddr_in))) < 0) {
		perror("sendto"); return -1;
	}

	buf[sentlen] = 0;
	printf("Sent %s to %s %d\n",
			buf, inet_ntoa(server.sin_addr), ntohs(server.sin_port));

	if(close(sockfd) < 0) {
		perror("close"); return -1;
	}

	return 0;
}
