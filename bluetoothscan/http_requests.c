#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include "http_requests.h"

#define SA      struct sockaddr
#define MAXLINE 5000
#define MAXSUB  5000
#define LISTENQ         1024
extern int h_errno;


void createHTTP(char *hostname, char *page, char *dataToSend, int port){
	//structure describing an internet socket address
	struct sockaddr_in servaddr;
	char **pptr;
	char str[100];

	//description of data base entry for a single host
	struct hostent *hptr;
	//if hostname is NULL throw error & exit
	if ((hptr = gethostbyname(hostname)) == NULL) {
		fprintf(stderr, " gethostbyname() error for host: %s: %s",
			hostname, hstrerror(h_errno));
		exit(1);
	}
	//printf("\nhostname: %s\n", hptr->h_name);

	//check the address type is correct & the host address list is not empty
	if (hptr->h_addrtype == AF_INET && (pptr = hptr->h_addr_list) != NULL){
	}else{
		fprintf(stderr, "Error call inet_ntop \n");
	}

	//convert ipv4/ipv6 form binary to text form & print
	//printf("address: %s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));

	//create a new socket
	//arg1 = address domain, arg2 = type of socket, arg3 = protocol - always 0 so OS choses protocol (TCP for stream sockets or UDP for datagram sockets)
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr)); //bzero() - fills area pointed with zero-values bytes
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port); //htons() (host to network short) converts values between host + network byte orders
	inet_pton(AF_INET, str, &servaddr.sin_addr); //convert IPv4 & IPv6 addresses from text to binary form

	//connect the socket & process the request
	connect(sock, (SA *) & servaddr, sizeof(servaddr));
	processHTTP(sock, hostname, page, dataToSend);

	//clear memory
	dataToSend = NULL;
	pptr = NULL;
	hptr = NULL;
	sock = 0;
	port = 0;
	page = NULL;
	hostname = NULL;

	memset(&servaddr, 0, sizeof(servaddr));

	memset(&str[0], 0, sizeof(str));

	//close the socket
	close(sock);
}

ssize_t processHTTP(int sock, char *hostname, char *page, char *dataToSend){
	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	ssize_t n;
	snprintf(sendline, MAXSUB,
		 "POST %s HTTP/1.0\r\n"
		 "Host: %s\r\n"
		 "Content-type: text/plain\r\n"
		 "Content-length: %d\r\n\r\n"
		 "%s", page, hostname, strlen(dataToSend), dataToSend);

	write(sock, sendline, strlen(sendline));
	while ((n = read(sock, recvline, MAXLINE)) > 0) {
		//recvline[n] = '\0';
		//printf("%s", recvline);
	}

	sock = 0;
	hostname = NULL;
	page = NULL;
	dataToSend = NULL;
	//memset(&n, 0, sizeof(n));
	memset(&sendline[0], 0, sizeof(sendline));
	memset(&recvline[0], 0, sizeof(recvline));
	return n;
}

