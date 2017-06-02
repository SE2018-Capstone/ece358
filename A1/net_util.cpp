/**
 * @brief: ECE358 network utility functions
 * @author: Mahesh V. Tripunitara
 * @file: net_util.c 
 * NoTES: code extra comments added by yqhuang@uwaterloo.ca
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "net_util.h"
#include "structs.h"

int read_sock(int sockfd, char* buf, int requested_size) {
	memset(buf, 0, requested_size);
	ssize_t recvlen;
	int bytes_received = 0;
	while (bytes_received < requested_size) {
		if((recvlen = recv(sockfd, buf + bytes_received, requested_size - bytes_received, 0)) < 0) {
			perror("recv");
			exit(-1);
		} else if (recvlen == 0) {
			return 0;
		}
		bytes_received += recvlen;
	}
	return bytes_received;
}

int send_sock(int sockfd, char* buf, int buf_size) {
	ssize_t sendlen;
	int bytes_sent = 0;
	while (bytes_sent < buf_size) {
		if((sendlen = send(sockfd, buf + bytes_sent, buf_size - bytes_sent, 0)) < 0) {
			perror("send");
			exit(-1);
		}
		bytes_sent += sendlen;
	}
	return bytes_sent;
}

std::string sockfd_to_str(int sockfd) {
	sockaddr_in addr;
	socklen_t alen = sizeof(struct sockaddr_in);
	if (getsockname(sockfd, (struct sockaddr *)&addr, &alen) < 0) {
		perror("getsockname"); return (char *) "ERROR";
	}

	std::ostringstream sstream;
	sstream << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port);
	return sstream.str();
}

struct sockaddr_in create_sockaddr_in(char* ip, uint32_t port) {
  struct sockaddr_in server;
  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  if (!inet_aton(ip, &(server.sin_addr))) {
    perror("invalid server-ip"); exit(-1);
  }
  server.sin_port = port;
  return server;
}

int connect_to_peer(struct sockaddr_in peer_server) {
  INFO("Connecting to peer at %s %d\n", inet_ntoa(peer_server.sin_addr), ntohs(peer_server.sin_port));

	int peer_sockfd = -1;
	if((peer_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("peer socket"); return -1;
	}
	if(connect(peer_sockfd, (struct sockaddr *)&peer_server, sizeof(struct sockaddr_in)) < 0) {
		fprintf(stderr, "Error: no such peer\n");
		return -1;
	}

	fd_set_nonblocking(peer_sockfd);

	return peer_sockfd;
}

int fd_set_nonblocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		perror("fcntl"); return -1;
	}
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) == -1) {
		perror("fcntl"); return -1;
	}
	return 0;
}

// Taken from http://wiki.linuxquestions.org/wiki/Fork_off_and_die
void daemonize() {
  int pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Can't fork.");
    exit(1);
  } else if (pid == 0) { // child
    setsid(); // release from parent
    int pid2 = fork();
    if (pid2 < 0) fprintf(stderr, "Can't fork after releasing.\n");
    else if (pid2 > 0) exit(0); // parent
    else { // Now running under init
      close(0); // Close stdin
      umask(0); chdir("/");
    }
  } else { // parent
    exit(0);
  }
}


/**
 * @brief: get a non-loopback IP address
 */

uint32_t getPublicIPAddr() 
{
	struct ifaddrs *ifa;

	if(getifaddrs(&ifa) < 0) {
		perror("getifaddrs"); exit(0);
	}

	struct ifaddrs *c;
	for(c = ifa; c != NULL; c = c->ifa_next) {
		if(c->ifa_addr == NULL) continue;
		if(c->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in a;

			memcpy(&a, (c->ifa_addr), sizeof(struct sockaddr_in));
			char *as = inet_ntoa(a.sin_addr);
#ifdef _DEBUG_
			printf("%s\n", as);
#endif // _DEBUG_

			int apart;
			sscanf(as, "%d", &apart);
			if(apart > 0 && apart != 127) {
				freeifaddrs(ifa);
				return (a.sin_addr.s_addr);
			}
		}
	}

	freeifaddrs(ifa);
	return 0;
}


/* 
 * @brief: a wrapper to bind that tries to bind() to a port in the
 *         range PORT_RANGE_LO - PORT_RANGE_HI, inclusive, 
 *         if the provided port is 0.
 *         Or else, it will try to just call bind instead.
 *
 * PRE: addr is an in-out parameter. That is, addr->sin_family and
 *      addr->sin_addr are assumed to have been initialized correctly 
 *      before the call.
 *      If addr->sin_port is not 0, it will try to bind to the provided port.
 *
 * @param sockfd -- the socket descriptor to which to bind
 * @param addr -- a pointer to struct sockaddr_in. 
 *                mybind() works for AF_INET sockets only.
 * @return int -- negative return means an error occurred, else the call succeeded.
 *
 * POST: Up on return, addr->sin_port contains, in network byte order, 
 *       the port to which the call bound sockfd.
 */
int mybind(int sockfd, struct sockaddr_in *addr) {
	if(sockfd < 1) {
		fprintf(stderr, "mybind(): sockfd has invalid value %d\n", sockfd);
		return -1;
	}

	if(addr == NULL) {
		fprintf(stderr, "mybind(): addr is NULL\n");
		return -1;
	}

	// if(addr->sin_port != 0) {
	//     fprintf(stderr, "mybind(): addr->sin_port is non-zero. Perhaps you want bind() instead?\n");
	//     return -1;
	// }

	if(addr->sin_port != 0) {
		if(bind(sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr_in)) < 0) {
			fprintf(stderr, "mybind(): cannot bind to port %d\n", addr->sin_port);
			return -1;
		}
		return 0;
	}

	unsigned short p;
	for(p = PORT_RANGE_LO; p <= PORT_RANGE_HI; p++) {
		addr->sin_port = htons(p);
		int b = bind(sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
		if(b < 0) {
			continue;
		}
		else {
			break;
		}
	}

	if(p > PORT_RANGE_HI) {
		fprintf(stderr, "mybind(): all bind() attempts failed. No port available...?\n");
		return -1;
	}

	/* Note: upon successful return, addr->sin_port contains, 
	 *     in network byte order, the  port to which we successfully bound. 
	 */
	return 0;
}


char readYorN() {
	char ret = EOF;
	int throwaway;
	while((throwaway = getchar()) != '\n')
		if(ret == EOF) ret = (char)throwaway;

	return ret;
}

/* Cycle through IP addresses. Let user pick one. */
int pickServerIPAddr(struct in_addr *srv_ip) {
	if(srv_ip == NULL) return -1;
	memset(srv_ip, 0, sizeof(struct in_addr));

	struct ifaddrs *ifa;
	if(getifaddrs(&ifa) < 0) {
		perror("getifaddrs"); exit(-1);
	}

	char c;
	for(struct ifaddrs *i = ifa; i != NULL; i = i->ifa_next) {
		if(i->ifa_addr == NULL) continue;
		if(i->ifa_addr->sa_family == AF_INET) {
			memcpy(srv_ip, &(((struct sockaddr_in *)(i->ifa_addr))->sin_addr), sizeof(struct in_addr));
			printf("Pick server-ip ");
			printf("%s [y/n]: ", inet_ntoa(*srv_ip));
			c = readYorN();
			if(c == 'Y' || c == 'y') {
				freeifaddrs(ifa);
				return 0;
			}
		}
	}

	/* Pick all IPs */
	printf("Pick server-ip 0.0.0.0 (all)? [y/n]: ");
	c = readYorN();
	if(c == 'Y' || c == 'y') {
		srv_ip->s_addr = htonl(INADDR_ANY);
		return 0;
	}

	/* No ip address picked. exit() */
	freeifaddrs(ifa);
	printf("You picked none of the options. Exiting...\n");
	exit(0);
}
