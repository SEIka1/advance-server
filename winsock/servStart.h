#pragma once

#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

int server_init();
extern void UpdateServerStatus(const wchar_t* status);