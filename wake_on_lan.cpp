// C program to remotely Power On a PC over the
// internet using the Wake-on-LAN protocol.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>

#include <iostream>
#include <stdexcept>

#define INC_MAC_ADDRESS "Incorrect MAC address"
#define INC_IP_ADDRESS "Incorrect IP address"

inline std::size_t cas(const char *arr) {
	// Returns length of char array
	std::size_t count = 0;
	while (arr[count] != '\0') ++count;
	return count;
}

int to_hex(const char &c) {
	// Converts chars to hex (integers)
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}


int main(int argc, char *argv[]) {
	// Checking if MAC has been provided
	if (argc == 1) throw std::invalid_argument("MAC address must be the first argument");
	
	// Checking MAC length
	if (cas(argv[1]) != 17) throw std::invalid_argument(INC_MAC_ADDRESS);
	

	int i;
	unsigned char toSend[102], mac[6]{0};

	// Filling MAC address
	for (i = 0; i < 17; ++i) {
		// Skipping delimeters
		if (i % 3 == 2) continue;

		// Checking MAC chars
		if (to_hex(argv[1][i]) == -1) throw std::invalid_argument(INC_MAC_ADDRESS);
		
		mac[i/3] = mac[i/3] == 0 ? to_hex(argv[1][i]) * 16 : mac[i/3] + to_hex(argv[1][i]);
	}

	const char *IP_ADDR = (argc == 3 ? argv[2] : "255.255.255.255");

	// Checking IP length
	if (cas(IP_ADDR) < 7 || cas(IP_ADDR) > 15) throw std::invalid_argument(INC_IP_ADDRESS);

	// Checking IP chars
	for (i = 0; i < cas(IP_ADDR); ++i)
		if (!(isdigit(IP_ADDR[i]) || IP_ADDR[i] == '.')) 
			throw std::invalid_argument(INC_IP_ADDRESS);
	
	struct sockaddr_in udpClient, udpServer;
	int broadcast = 1 ;

	// UDP Socket creation
	int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	// Manipulating the Socket
	if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST,
				&broadcast, sizeof broadcast) == -1)
	{
		perror("setsockopt (SO_BROADCAST)");
		exit(EXIT_FAILURE);
	}
	udpClient.sin_family = AF_INET;
	udpClient.sin_addr.s_addr = INADDR_ANY;
	udpClient.sin_port = 0;

	//Binding the socket
	bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient));

	for (i=0; i<6; i++)
		toSend[i] = 0xFF;

	for (i=1; i<=16; i++)
		memcpy(&toSend[i*6], &mac, 6*sizeof(unsigned char));

	udpServer.sin_family = AF_INET;

	// Broadcast address
	udpServer.sin_addr.s_addr = inet_addr(IP_ADDR);
	udpServer.sin_port = htons(9);

	sendto(
		udpSocket,
		&toSend,
		sizeof(unsigned char) * 102,
		0,
		(struct sockaddr*)&udpServer,
		sizeof(udpServer)
	);

	std::cout << "Sending magic packet to:\nMAC: "
			  << argv[1] << "\nIP: "
			  << IP_ADDR << std::endl;

	return 0;
}
