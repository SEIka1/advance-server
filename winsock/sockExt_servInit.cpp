#include "./server_include_init.h"
#include "./server_resource_init.h"
#include "./servStart.h"

int server_init() {
    UpdateServerStatus(L"Инициализация WinSock...");

    int erStat;
    in_addr ip_to_num;
    erStat = inet_pton(AF_INET, SERVER_IP, &ip_to_num);

    if (erStat <= 0) {
        UpdateServerStatus(L"Ошибка в преобразовании IP адреса");
        return 1;
    }

    WSADATA wsData;
    erStat = WSAStartup(MAKEWORD(2, 2), &wsData);

    if (erStat != 0) {
        UpdateServerStatus(L"Ошибка инициализации WinSock");
        return 1;
    }

    UpdateServerStatus(L"Создание сокета...");
    SOCKET ServSock = socket(AF_INET, SOCK_STREAM, 0);

    if (ServSock == INVALID_SOCKET) {
        UpdateServerStatus(L"Ошибка создания сокета");
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }

    UpdateServerStatus(L"Привязка сокета...");
    sockaddr_in servInfo;
    ZeroMemory(&servInfo, sizeof(servInfo));
    servInfo.sin_family = AF_INET;
    servInfo.sin_addr = ip_to_num;
    servInfo.sin_port = htons(SERVER_PORT_NUM);

    erStat = bind(ServSock, (sockaddr*)&servInfo, sizeof(servInfo));
    if (erStat != 0) {
        UpdateServerStatus(L"Ошибка привязки сокета");
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }

    UpdateServerStatus(L"Начало прослушивания...");
    erStat = listen(ServSock, SOMAXCONN);
    if (erStat != 0) {
        UpdateServerStatus(L"Ошибка при начале прослушивания");
        closesocket(ServSock);
        WSACleanup();
        return 1;
    }

    UpdateServerStatus(L"Сервер запущен и ожидает подключений...\r\nIP: ");
    wchar_t statusMsg[256];
    swprintf(statusMsg, 256, L"Сервер запущен\r\nIP: %hs\r\nПорт: %d\r\nОжидание подключений...",
        SERVER_IP, SERVER_PORT_NUM);
    UpdateServerStatus(statusMsg);


    while (true) {
        sockaddr_in clientInfo;
        ZeroMemory(&clientInfo, sizeof(clientInfo));
        int clientInfo_size = sizeof(clientInfo);

        SOCKET ClientConn = accept(ServSock, (sockaddr*)&clientInfo, &clientInfo_size);
        if (ClientConn == INVALID_SOCKET) {
            UpdateServerStatus(L"Ошибка подключения клиента");
            continue;
        }

        char clientIP[22];
        inet_ntop(AF_INET, &clientInfo.sin_addr, clientIP, INET_ADDRSTRLEN);
        swprintf(statusMsg, 256, L"Подключен новый клиент\r\nIP клиента: %hs", clientIP);
        UpdateServerStatus(statusMsg);

    }
}