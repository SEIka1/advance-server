#include "./client_include_init.h"
#include "./client_resource_init.h"


int client_init()
{

	int erStat;

	in_addr ip_to_num;
	inet_pton(AF_INET, SERVER_IP, &ip_to_num);


	WSADATA wsData;
	erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (erStat != 0) {
		std::cerr << "Error WinSock version initializaion: ";
		std::cerr << WSAGetLastError();
		return 1;
	}
	else
		std::cout << "WinSock initialization is OK" << std::endl;

	SOCKET ClientSock = socket(AF_INET, SOCK_STREAM, 0);

	if (ClientSock == INVALID_SOCKET) {
		std::cerr << "Error initialization socket: " << WSAGetLastError() << std::endl;
		closesocket(ClientSock);
		WSACleanup();
	}
	else
		std::cout << "Client socket initialization is OK" << std::endl;

	sockaddr_in servInfo;

	ZeroMemory(&servInfo, sizeof(servInfo));

	servInfo.sin_family = AF_INET;
	servInfo.sin_addr = ip_to_num;
	servInfo.sin_port = htons(SERVER_PORT_NUM);

	erStat = connect(ClientSock, (sockaddr*)&servInfo, sizeof(servInfo));

	if (erStat != 0) {
		std::cerr << "Connection to Server is FAILED: " << WSAGetLastError() << std::endl;
		closesocket(ClientSock);
		WSACleanup();
		return 1;
	}
	else
		std::cout << "Connection established SUCCESSFULLY" << std::endl;


	std::vector <char> servBuff(SIZE_OF_BUFFER), clientBuff(SIZE_OF_BUFFER);
	short packet_size = 0;

	while (true) {

		std::cout << "Your (Client) message to Server: ";
		fgets(clientBuff.data(), clientBuff.size(), stdin);

		if (clientBuff[0] == 'q' && clientBuff[1] == 'u' && clientBuff[2] == 'i' && clientBuff[3] == 't') {
			shutdown(ClientSock, SD_BOTH);
			closesocket(ClientSock);
			WSACleanup();
			return 0;
		}

		packet_size = send(ClientSock, clientBuff.data(), clientBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			std::cerr << "Can't send message to Server. Error # " << WSAGetLastError() << std::endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}

		packet_size = recv(ClientSock, servBuff.data(), servBuff.size(), 0);

		if (packet_size == SOCKET_ERROR) {
			std::cerr << "Can't receive message from Server. Error # " << WSAGetLastError() << std::endl;
			closesocket(ClientSock);
			WSACleanup();
			return 1;
		}
		else
			std::cout << "Server message: " << servBuff.data() << std::endl;

	}

	closesocket(ClientSock);
	WSACleanup();

	return 0;
}