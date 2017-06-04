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
#define INFO(...) fprintf(stderr, "\x1b[34m %3i: ", debug_id); fprintf( stderr, __VA_ARGS__); fprintf(stderr, "\x1b[0m");
#define INFO_RED(...) fprintf(stderr, "\x1b[31m %3i: ", debug_id); fprintf( stderr, __VA_ARGS__); fprintf(stderr, "\x1b[0m");
#define INFO_YELLOW(...) fprintf(stderr, "\x1b[33m %3i: ", debug_id); fprintf( stderr, __VA_ARGS__); fprintf(stderr, "\x1b[0m");
#else
#define INFO(...) do {} while (0)
#define INFO_RED(...) do {} while (0)
#define INFO_YELLOW(...) do {} while (0)
#endif

// functions
bool sockaddr_equals(struct sockaddr_in server1, struct sockaddr_in server2);
int read_sock(int sockfd, char* buf, int requested_size);
int send_sock(int sockfd, char* buf, int buf_size);
std::string sockaddr_to_str(const struct sockaddr_in &sockaddr);
std::string sockfd_to_str(int sockfd);
struct sockaddr_in create_sockaddr_in(char* ip, uint32_t port);
int connect_to_peer(struct sockaddr_in peer_server);
int fd_set_nonblocking(int fd);
void daemonize();
uint32_t getPublicIPAddr();
int mybind(int sockfd, struct sockaddr_in *addr); 
int pickServerIPAddr(struct in_addr *srv_ip);

#endif // ! NET_UTIL_H_
