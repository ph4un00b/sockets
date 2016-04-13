#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // htonl, htons
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h> // gethostbyname
#include <sys/socket.h>

#define MAXRCVLEN 500
#define PORTNUM 2300
#define DATA "LOL JAMON!"
int 
main(int argc, char** argv)
{
	char buffer[MAXRCVLEN + 1];
	int len = 0;
	int my_socket = 0;
	struct sockaddr_in destination;
	struct hostent* hp;

	// create socket
	my_socket = socket(AF_INET, SOCK_STREAM, 0);

	if ( my_socket < 0 )
 	{
 		perror ("my_socket err");
 		exit (EXIT_FAILURE);
 	}

	memset(&destination, 0, sizeof(destination)); 

	destination.sin_family = AF_INET;

	hp = gethostbyname(argv[1]);

	if ( !hp )
	{
		perror ("gethostbyname err");
		close(my_socket); 
 		exit (EXIT_FAILURE);
	}

	// http://linux.die.net/man/3/htonl
	// destination.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // ip in Big Endian format

	memcpy(&destination.sin_addr, hp->h_addr, hp->h_length);

	destination.sin_port = htons(PORTNUM); // port

	if ( connect(my_socket, (struct sockaddr *)&destination, sizeof(struct sockaddr)) < 0 )
	{
		perror ("connect err");
		close(my_socket);
 		exit (EXIT_FAILURE);
	}

	if ( send(my_socket, DATA, sizeof(DATA), 0) < 0)
	{
		perror ("send err");
		close(my_socket);
 		exit (EXIT_FAILURE);
	}

	printf("Sent msg to server: %s\n", DATA);

	// UDP: recvfrom
	len = recv(my_socket, buffer, MAXRCVLEN, 0);
	buffer[len] = '\0'; // null terminate
	printf("Received %s (%d bytes).\n", buffer, len);

	close(my_socket);
	return 0;
}
