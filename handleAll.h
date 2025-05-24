#ifndef HANDLEALL
#define HANDLEALL
#include <iostream>
#include <fstream>
#include <string>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include "ProgressBar.h"
#include "mkdir.h"
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	std::cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<std::endl;
constexpr size_t BUF = 64 * 1024;          // 分块传输大小

bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename);
bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename, const std::string& account);
bool sendAll(SOCKET s, char* p, size_t len, FILE* stream);
std::string getfilename(const std::string& str);
bool deleteFile(const std::string& filename);

#endif