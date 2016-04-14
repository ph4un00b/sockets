#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // fork
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h> // gethostbyname

	//									UDP
	//
	//
	//	|    - SERVER      |            | - CLIENT 1    | - CLIENT 2    |
	//  | CREATE SOCKET	   |            |               |
	//  |				   |			|				|
	//  | BIND PORT 	   |            | 				|
	//  |		 		   |            |				|
	//	|				   |			| CREATE SOCKET |
	// 
	//  | RECEIVE DATAGRAM | <--------- | SEND DATAGRAM |
	//	|				   |			|				|
	//  | SEND REPLY	   | ---------> | RECEIVE REPLY |	
	//	
	//	| RECEIVE DATAGRAM | <------------------------- | SEND DATAGRAM |
	//  |				   |							|				|
	//	| SEND REPLY       | -------------------------> | RECEIVE REPLY |
	//
	//
	//


	// domain: AF_UNIX, AF_INET. AF_INET6
	// type: SOCK_STREAM, SOCK_DGRAM
	// protocol: 0, system selects based on domain && type
	
	//socket(AF_INET, SOCK_STREAM, 0); 

	// SENDING DATAGRAM
	// sock: descriptor
	// buff: data to send
	// count: sizeof(buff)
	// flags: delivery options, usually 0
	// addr: datagram destination (struct sockaddr_in) -- diff
	// len: sizeof(struct sockaddr_in)
	
	//sendto(sock, buff, count, flags, addr, len);

	// RECEIVING DATAGRAM (in bytes)
	// sock: descriptor
	// buff: data to send
	// count: sizeof(buff)
	// flags: delivery options, usually 0
	// addr: datagram sender (struct sockaddr_in) -- diff
	// len: sizeof(struct sockaddr_in)

	// recvfrom(sock, buff, count, flags, addr, len);



	// 						RETURN TFTP PACKET FORMAT
	//
	//	| OPCODE 3 = DATA | BLOCK NUMBER | DATA 		 |
	//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |
	//

	// 						ACK TFTP PACKET FORMAT
	//
	//	| OPCODE 4 = ACK  | BLOCK NUMBER | DATA 		 |
	//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |
	//

	// 						ERROR TFTP PACKET FORMAT
	//
	//	| OPCODE 5 = ERR  | ERROR NUMBER | ERROR MSG	 | '\0' |
	//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |		|
	//


	//
	//	| SERVER | <--------------------- | CLIENT |
	//  |		 | RRQ ON PORT 69         |
	//
	//  | SERVER | ---------------------> | CLIENT |
	//  |		 | UDP SOCK WITH N OPCODE |
	//
	//  | SERVER | <--------------------- | CLIENT |
	//  |		 | ACK 					  |
	//

	#define TFTP_PORT	69
	#define BUFF_SIZE 	600
	#define MODE	 	"octec"

	#define OP_RRQ	 	1
	#define OP_DATA	 	3
	#define OP_ACK	 	4
	#define OP_ERR	 	5

int main(int argc, char *argv[])
{
	int sock;							// descriptor
	struct sockaddr_in server;			// server address
	struct hostent* server_host_info;	// server info
	char buff[BUFF_SIZE];
	char* byte_ptr;
	int count;
	int server_len;

	if ( argc != 3 )
	{
		fprintf(stderr, "usage: %s hostname filename\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* CREATE SOCKET */

	// domain: 	AF_UNIX, AF_INET. AF_INET6
	// type: 	SOCK_STREAM, SOCK_DGRAM
	// protocol: 0, system selects based on domain && type
	sock = socket(AF_INET, SOCK_DGRAM, 0); 

	// get server's address
	server_host_info = gethostbyname(argv[1]);

	if (!server_host_info)
	{
		fprintf(stderr, "no host? %s\n", argv[1]);
		exit(2);
	}

	server.sin_family = AF_INET;

	memcpy(&server.sin_addr.s_addr, 
		server_host_info->h_addr, 
		server_host_info->h_length);
	
	server.sin_port = htons(TFTP_PORT);

	// 						TFTP PACKET FORMAT
	// 
	//  RRQ = READ REQUEST
	//
	//	| IP HEADER | UDP HEADER | OPCODE 1 = RRQ | FILENAME | '\0' | MODE   | '\0' |
	//  | 20 BYTES  | 8 BYTES    | 2 BYTES        | N BYTES  |      | N BYTES|		|
	//

	*(short *)buff = htons(OP_RRQ);
	byte_ptr = buff + 2;

	strcpy(byte_ptr, argv[2]); // filename
	byte_ptr += strlen(argv[2]) + 1; 

	strcpy(byte_ptr, MODE);
	byte_ptr += strlen(MODE) + 1;

	//
	//	| SERVER | <--------------------- | CLIENT |
	//  |		 | RRQ ON PORT 69         |

			// sock: descriptor
			// buff: data to send
			// count: sizeof(buff)
			// flags: delivery options, usually 0
			// addr: datagram destination (struct sockaddr_in) -- diff
			// len: sizeof(struct sockaddr_in)

	count = sendto(sock, 
		buff, 
		byte_ptr - buff, 
		0,
		(struct sockaddr*)&server, 
		sizeof(server));

	printf("count server: %d\n", count);
	do
	{
		// 						RETURN TFTP PACKET FORMAT
		//
		//	| OPCODE 3 = DATA | BLOCK NUMBER | DATA 		 |
		//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |
		//

		server_len = sizeof( server );

		// RECEIVING DATAGRAM (in bytes)
		count = recvfrom(sock, 
			buff, 
			BUFF_SIZE, 
			0, 
			(struct sockaddr*)&server, // get endpoint where data came from
			(socklen_t*) &server_len);

		printf("count received: %d\n", count);

		if ( ntohs( *(short *)buff ) == OP_ERR )
		{
			// 						ERROR TFTP PACKET FORMAT
			//
			//	| OPCODE 5 = ERR  | ERROR NUMBER | ERROR MSG	 | '\0' |
			//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |		|
			//
			fprintf(stderr, "rcat%s\n", buff + 4);
		}
		else
		{ 
			// file descriptor: 1, 2
			write(1, buff + 4, count - 4);

			// 						ACK TFTP PACKET FORMAT
			//
			//	| OPCODE 4 = ACK  | BLOCK NUMBER | DATA 		 |
			//  | 2 BYTES         | 2 BYTES      | 0 - 512 BYTES |
			//
			*(short *)buff = htons(OP_ACK);
			sendto(sock, 
				buff, 
				4, // sending 4 bytes packet
				0, 
				(struct sockaddr*)&server, // send to endpoint address
				sizeof(server));	
		}
		
	} while (count == 516);
	
	return 0;
}
