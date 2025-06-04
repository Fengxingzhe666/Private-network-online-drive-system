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

// 自定义的64位主机字节序和网络字节序的相互转换，方便跨平台（socket中只有32位）
inline unsigned long long ntoh64(unsigned long long val)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	return (((unsigned long long)htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
#else __BYTE_ORDER == __BIG_ENDIAN
	return val;
#endif
}

inline unsigned long long hton64(unsigned long long val)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
	return (((unsigned long long)htonl((int)((val << 32) >> 32))) << 32) | (unsigned int)htonl((int)(val >> 32));
#else __BYTE_ORDER == __BIG_ENDIAN
	return val;
#endif
	//参考网址：http://www.cppblog.com/aa19870406/archive/2012/06/20/179517.html
}

#endif