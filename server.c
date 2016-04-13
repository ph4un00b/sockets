#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#define PORTNUM 2300

int
main()
{
	char buffer[1024 + 1];
	int rval;
	char* msg = "Hello ham, I will wait for you...";

 	struct sockaddr_in guest;	
 	struct sockaddr_in server;

 	int my_socket = 0; // listener
 	socklen_t socket_size = sizeof(struct sockaddr_in);

 	memset(&server, 0, sizeof(server)); 


 	// create socket
 	// UDP: SOCK_DGRAM
 	my_socket = socket(AF_INET, SOCK_STREAM, 0); // TCP

 	if ( my_socket < 0 )
 	{
 		perror ("my_socket err");
 		exit (EXIT_FAILURE);
 	}

 	server.sin_family = AF_INET; // TCP/IP type
	server.sin_addr.s_addr = htonl(INADDR_ANY); // any interface
	server.sin_port = htons(PORTNUM); 

 	// call bind
 	if ( bind(my_socket, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0 )
 	{
 		perror ("bind err");
 		exit (EXIT_FAILURE);
 	}


 	// listen
 	listen(my_socket, 1); // queue of up to 4 pending connection

	// accept
 	int new_socket = accept(my_socket, (struct sockaddr *)&guest, &socket_size);
	
	while(new_socket)
	{
		printf("Incoming connection from %s - sending welcome\n", 
			inet_ntoa(guest.sin_addr));

		// UDP: sendto
		send(new_socket, msg, strlen(msg), 0); 

		while (1)
		{
			// wait for receive
			memset(buffer, 0, sizeof(buffer));
			rval = recv(new_socket, buffer, sizeof(buffer), 0);

			if ( rval < 0 )	
			{
				perror ("reading err");
				return 0;
			}
			// else if ( !rval )
			// {
			// 	printf ("ending connection\n");
			// }
			else if ( !strncmp("exit", buffer, 4) )
			{
				strcpy(buffer, "adios\n");
				printf ("buffer: %s\n", buffer);
				send(new_socket, buffer, strlen(buffer), 0); 
				close(new_socket);
			}
			else if ( !strncmp("ls", buffer, 2) )
			{
				strcpy(buffer, "all users\n");
				printf ("buffer: %s\n", buffer);
				send(new_socket, buffer, strlen(buffer), 0); 
			}
			else 
			{
				buffer[rval] = '\0'; // null terminate
				printf ("Received message: %s, bytes: %d\n", buffer, rval);
				
			}
		}
		
		printf ("new_socket\n");
		new_socket = accept(my_socket, (struct sockaddr *)&guest, &socket_size);
	} 	
 	// if ( new_socket < 0 )	
 	// {
 	// 	perror ("accept err");
 	// 	exit (EXIT_FAILURE);
 	// }
 	// else
 	// {
 		
 	// }

 	close(my_socket);
	return 0;
}
