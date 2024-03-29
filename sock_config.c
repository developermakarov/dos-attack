/*
 * =====================================================================================
 *
 *       Filename:  sock_config.c
 *
 *    Description:  Configure the shared parts of the attacks
 *
 * =====================================================================================
 */
#include "sock_config.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Opens a socket for TCP
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int get_tcp_sock_fd()
{
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    if(fd < 0) {
        perror("cannot open socket");
        return -1;
    }
    return fd;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Opens a socket for UDP
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int get_udp_sock_fd()
{
    int fd = socket(AF_INET, SOCK_DGRAM,0);

    if(fd < 0) {
        perror("cannot open socket");
        return -1;
    }
    return fd;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Configure the socket
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
struct sockaddr_in configure_sock(int port, char *hostname)
{
    struct sockaddr_in servaddr;

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(hostname);
    servaddr.sin_port = htons(port);

    return servaddr;
}

int check_args(char* hostname, char* message, int port, char* source_ip, char* attack_type)
{
    if (!*hostname) {
        printf ("usage: -p port -h hostname -m message\n");
        return -1;
    }
    if (!*message && !strncmp(attack_type, "udp_spam", 8)) {
        printf ("usage: -p port -h hostname -m message\n");
        return -1;
    }
    if (!port) {
        printf ("usage: -p port -h hostname -m message\n");
        return -1;
    }
    if (!*source_ip && !strncmp(attack_type, "syn_flood", 9)) {
        printf ("usage: -p port -h hostname -s source_ip\n");
        return -1;
    }
    return 0;
}
