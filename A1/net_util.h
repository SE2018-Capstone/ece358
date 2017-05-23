/**
 * @brief: ECE358 network utility functions 
 * @author: ECE358 Teaching Staff
 * @file: net_util.h 
 * @date: 2017-05-02
 */

#ifndef NET_UTIL_H_
#define NET_UTIL_H_

//includes
#include <netinet/in.h>
#include <string>

// defines
#define PORT_RANGE_LO 10000
#define PORT_RANGE_HI 11000

#ifdef DEBUG
#define INFO(...) fprintf(stderr, "\x1b[34m "); fprintf( stderr, __VA_ARGS__); fprintf(stderr, "\x1b[0m");
#else
#define INFO(...) do {} while (0)
#endif

// functions
int read_sock(int sockfd, char* buf, int requested_size);
int send_sock(int sockfd, char* buf, int buf_size);
std::string sockfd_to_str(int sockfd);
int connect_to_peer(char* ip, uint32_t port, struct sockaddr_in* peer_server);
int fd_set_nonblocking(int fd);
void daemonize();
uint32_t getPublicIPAddr();
int mybind(int sockfd, struct sockaddr_in *addr); 
int pickServerIPAddr(struct in_addr *srv_ip);

#endif // ! NET_UTIL_H_
