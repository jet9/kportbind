#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <errno.h>
#include <strings.h>

void usage(char *prog_name) {
	printf("Usage: %s port_number\n", prog_name);
	exit(1);
}

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in self;
	int i;
	unsigned short int port;

	if (argc < 2) {
		usage(argv[0]);
	}
	
	port = atoi(argv[1]);
	if (port == 0) {
		usage(argv[0]);
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		perror("socket()");
		return errno;
	}

	bzero(&self, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = INADDR_ANY;

	if ( bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0 ) {
		perror("bind() error");
		return errno;
	}

	if ( listen(sockfd, 20) != 0 ) {
		perror("listen() error");
		return errno;
	}

	close(sockfd);
	return 0;
}
