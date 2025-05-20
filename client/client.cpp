//client.cpp                         --模拟客户端
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <memory>     //智能指针
#include<Winsock2.h>
#include<WS2tcpip.h>
#include "../ProgressBar.h"
#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	std::cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<std::endl;

using std::string;
constexpr int PORT = 5000;

int main()
{
	// 存储 WSAStartup 初始化信息的结构体
	WSADATA wsaData;
	// 初始化 Winsock，指定使用版本 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建客户端套接字，AF_INET=IPv4，SOCK_STREAM=TCP
	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		// 若创建失败，打印错误并退出
		std::cerr << "Creat SOCKET error" << std::endl;
		return -1;
	}

	// 存储服务器信息的地址结构
	struct sockaddr_in target;
	// 协议族：IPv4
	target.sin_family = AF_INET;
	// 目标端口号（要与服务器保持一致）
	target.sin_port = htons(PORT);
	// inet_pton: 将字符串形式的 IP("127.0.0.1") 转为网络字节序地址并存储到 sin_addr.s_addr
	string ip_str;
	std::cout << "Enter the server's ip address,such as 127.0.0.1 on this device itself." << std::endl;
	getline(std::cin, ip_str);
	// 如果你的编译器或环境不支持 inet_pton，可用 inet_addr("127.0.0.1")
	inet_pton(AF_INET, ip_str.c_str(), &target.sin_addr.s_addr);
	//target.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 连接到服务器，若失败返回 INVALID_SOCKET
	if (connect(client_socket, (struct sockaddr*)&target, sizeof target) == INVALID_SOCKET) {
		std::cerr << "Connection error, WSA " << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		return -1;
	}
	std::cout << "client connects to server successfully." << std::endl;

	// 进入循环，不断从控制台输入消息并发送给服务器
	while (true) {
		string sending_str, control_msg,filename;
		getline(std::cin,sending_str);
		control_msg = sending_str.substr(0, 3);
		filename = sending_str.substr(3);
		// 将输入的消息发送给服务器
		if (send(client_socket, sending_str.c_str(), sending_str.size(), 0) <= 0) {
			std::cout << "server disconnect." << std::endl;
			break;
		}
		// 客户端希望接收文件信息
		if (control_msg == "-r ") {
			// 接收服务器的回显消息（文件大小）
			char buffer2[8] = { 0 };
			uint32_t NetSize = 0;
			// 从服务器接收数据，当服务器断开或出错时，返回值 <= 0
			if (recv(client_socket, reinterpret_cast<char*>(&NetSize), sizeof(NetSize), 0) <= 0) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			uint32_t FileSize = ntohl(NetSize);
			std::cout << "File size: " << FileSize << " byte(s)" << std::endl;
			if (FileSize == 0) {
				std::cout << "Error! Server could not found the file. Or server refused to send an empty file." << std::endl;
				continue;
			}
			send(client_socket, "OK\0", 3, 0);
			char fp[BUF] = {};
			//std::unique_ptr<char> fp(new char[BUF]);
			if (recvAll(client_socket, fp, FileSize, filename))
				std::cout << std::endl << "Receive file Successfully." << std::endl;
		}
		// 客户端希望发送文件信息
		else if (control_msg == "-s ") {
			

		}
	}
	// 结束后关闭套接字
	closesocket(client_socket);
	return 0;
}
/*
代码说明，client.cpp：
同样调用 WSAStartup 初始化网络库，创建一个套接字，使用 connect() 连接到服务器的 127.0.0.1:9999
在循环里不停地从用户输入获取字符串并发送给服务器，然后等待服务器的回显信息并打印。
*/
