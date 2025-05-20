#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include<Winsock2.h>
#include<WS2tcpip.h>
#include "ProgressBar.h"
#pragma comment(lib, "ws2_32.lib")

constexpr size_t BUF = 256 * 1024;          // 分块传输大小

bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename);
bool sendAll(SOCKET s, const char* p, size_t len);
std::string getfilename(const std::string& str);