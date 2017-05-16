/*
 * @brief: a sample udp server program for ECE358 course project 
 * @file: us.c
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

#include "net_util.h"

int main() {
	struct in_addr srvip;
	if(pickServerIPAddr(&srvip) < 0) {
		fprintf(stderr, "pickServerIPAddr() returned error.\n");
		exit(-1);
	}

	int sockfd = -1;
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket"); return -1;
	}

	struct sockaddr_in server;
	bzero(&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	memcpy(&(server.sin_addr), &srvip, sizeof(struct in_addr)); // From above
	server.sin_port = 0; // Allow OS to pick port

	if(bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
		perror("bind"); return -1;
	}

	socklen_t alen = sizeof(struct sockaddr_in);
	if(getsockname(sockfd, (struct sockaddr *)&server, &alen) < 0) {
		perror("getsockname"); return -1;
	}

	printf("server starting at %s %d\n",
			inet_ntoa(server.sin_addr), ntohs(server.sin_port));

	size_t buflen = 10;
	char buf[buflen];
	ssize_t recvlen;
	struct sockaddr_in client;
	alen = sizeof(struct sockaddr_in);
	if((recvlen = recvfrom(sockfd, buf, buflen-1, 0,
					(struct sockaddr *)&client, &alen)) < 0) {
		perror("recvfrom"); return -1;
	}

	buf[recvlen] = 0; // ensure null-terminated string
	printf("Received the following %d-length string from %s %d: ",
			(int)recvlen, inet_ntoa(client.sin_addr),
			ntohs(client.sin_port));
	printf("%s\n", buf);

	if(close(sockfd) < 0) {
		perror("close"); return -1;
	}

	return 0;
}
