//==================================================== file = magicSend.c =====
//=  Program to send a Magic Packet to wake-up a sleeping target computer     =
//=============================================================================
//=    1) Compiles for WinSock                                                =
//=    2) A Magic Packet is a frame containing anywhere within its payload    =
//=       6 bytes of ones followed by sixteen repetitions of the target       =
//=       computer's MAC address.                                             =
//=    3) The target computer must be enabled for wake-up by a Magic Packet   =
//=    4) The target computer IP address and MAC address must be defined in   =
//=       the #define below                                                   =
//=---------------------------------------------------------------------------=
//=  Example execution:                                                       =
//=  Sending Magic Packet to target...                                        =
//=    Target IP address  = 131.247.3.77                                      =
//=    Target MAC address = 00-19-B9-32-BC-76                                 =
//=---------------------------------------------------------------------------=
//=  Build: bcc32 magicSend.c or cl magicSend.c wsock32.lib for Winsock       =
//=---------------------------------------------------------------------------=
//=  History:                                                                 =
//=    KJC (01/30/09) - Genesis                                               =
//=============================================================================
//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <windows.h>        // Needed for all Winsock stuff
#include <stdexcept>        // To throw errors

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM           9     // Port number
#define INC_MAC_ADDRESS "Incorrect MAC address"
#define INC_IP_ADDRESS "Incorrect IP address"

//----- Functions -------------------------------------------------------------
inline std::size_t cas(const char* arr) {
	// Returns length of char array
	std::size_t count = 0;
	while (arr[count] != '\0') ++count;
	return count;
}

int to_hex(const char& c) {
	// Converts chars to hex (integers)
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}

//===== Main program ==========================================================
void main(int argc, char* argv[])
{
	// Checking if MAC has been provided
	if (argc == 1) throw std::invalid_argument("MAC address must be the first argument");

	// Checking MAC length
	if (cas(argv[1]) != 17) throw std::invalid_argument(INC_MAC_ADDRESS);

	int i;
	char mac[6]{ 0 };

	// Filling MAC address
	for (i = 0; i < 17; ++i) {
		// Skipping delimeters
		if (i % 3 == 2) continue;

		// Checking MAC chars
		if (to_hex(argv[1][i]) == -1) throw std::invalid_argument(INC_MAC_ADDRESS);

		mac[i / 3] = mac[i / 3] == 0 ? to_hex(argv[1][i]) * 16 : mac[i / 3] + to_hex(argv[1][i]);
	}

	int MAC_BYTE1 = mac[0];
	int MAC_BYTE2 = mac[1];
	int MAC_BYTE3 = mac[2];
	int MAC_BYTE4 = mac[3];
	int MAC_BYTE5 = mac[4];
	int MAC_BYTE6 = mac[5];


	const char* IP_ADDR = (argc == 3 ? argv[2] : "255.255.255.255");

	// Checking IP length
	if (cas(IP_ADDR) < 7 || cas(IP_ADDR) > 15) throw std::invalid_argument(INC_IP_ADDRESS);

	// Checking IP chars
	for (i = 0; i < cas(IP_ADDR); ++i)
		if (!(isdigit(IP_ADDR[i]) || IP_ADDR[i] == '.'))
			throw std::invalid_argument(INC_IP_ADDRESS);

	WORD wVersionRequested = MAKEWORD(1, 1); // Stuff for WSA functions
	WSADATA wsaData;                         // Stuff for WSA functions
	int                  client_s;           // Client socket descriptor
	struct sockaddr_in   target_addr;        // Target Internet address
	int                  addr_len;           // Internet address length
	int                  pkt_len;            // Packet length
	char                 out_buf[1024];      // Output buffer for data
	int                  retcode;            // Return code

	// This stuff initializes winsock
	WSAStartup(wVersionRequested, &wsaData);

	// Create a client socket
	client_s = socket(AF_INET, SOCK_DGRAM, 0);
	if (client_s < 0)
	{
		printf("*** ERROR - socket() failed \n");
		exit(-1);
	}

	// Fill-in target address information
	target_addr.sin_family = AF_INET;
	target_addr.sin_port = htons(PORT_NUM);
	target_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

	// Load the Magic Packet pattern into the output buffer
	for (i = 0; i < 6; i++)
		out_buf[i] = 0xff;
	for (i = 0; i < 16; i++)
	{
		out_buf[(i + 1) * 6 + 0] = MAC_BYTE1;
		out_buf[(i + 1) * 6 + 1] = MAC_BYTE2;
		out_buf[(i + 1) * 6 + 2] = MAC_BYTE3;
		out_buf[(i + 1) * 6 + 3] = MAC_BYTE4;
		out_buf[(i + 1) * 6 + 4] = MAC_BYTE5;
		out_buf[(i + 1) * 6 + 5] = MAC_BYTE6;
	}
	pkt_len = 102;

	// Now send the Magic Packet to the target
	printf("Sending Magic Packet to target... \n");
	printf("  Target IP address  = %s \n", IP_ADDR);
	printf("  Target MAC address = %02X-%02X-%02X-%02X-%02X-%02X \n",
		MAC_BYTE1, MAC_BYTE2, MAC_BYTE3, MAC_BYTE4, MAC_BYTE5, MAC_BYTE6);
	retcode = sendto(client_s, out_buf, pkt_len, 0,
		(struct sockaddr*)&target_addr, sizeof(target_addr));
	if (retcode < 0)
	{
		printf("*** ERROR - sendto() failed \n");
		exit(-1);
	}

	// Wait for 0.5 seconds for the packet to be sent
	Sleep(500);

	// Close client socket and clean-up
	retcode = closesocket(client_s);
	if (retcode < 0)
	{
		printf("*** ERROR - closesocket() failed \n");
		exit(-1);
	}
	WSACleanup();
}
