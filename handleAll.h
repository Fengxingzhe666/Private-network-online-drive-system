#ifndef HANDLEALL
#define HANDLEALL
#ifdef _WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#define err(errMsg)	std::cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<std::endl;
#else
#include<unistd.h>
#include<arpa/inet.h>
#include <sys/types.h>       
#include <sys/socket.h>
#include<cstring>
#include <netdb.h>
#define SOCKET int
#define INVALID_SOCKET SOCKET (~0)
#define SOCKET_ERROR (-1)
#define err(errMsg) std::cout<<errMsg<<"line:"<<__LINE__<<std::endl;
#endif
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include "ProgressBar.h"
#include "mkdir.h"
#pragma comment(lib, "ws2_32.lib")

constexpr size_t BUF = 64 * 1024;          // 分块传输大小
constexpr int DEFAULT_STR_LENGTH = 128;    // 未知长度时，默认发送/接收字符串的长度

bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename);
bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename, const std::string& account);
bool recvevery(SOCKET s, char* p, size_t len, int flag);
bool sendAll(SOCKET s, char* p, size_t len, FILE* stream);
bool sendevery(SOCKET s, const char* p, size_t len, int flag);
std::string getfilename(const std::string& str);
bool deleteFile(const std::string& filename);

#endif