//client.cpp                         --模拟客户端
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include "../ProgressBar.h"
#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib")

using std::string;
constexpr int PORT = 5000;

int main()
{
#ifdef _WIN32
	// 存储 WSAStartup 初始化信息的结构体
	WSADATA wsaData;
	// 初始化 Winsock，指定使用版本 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

	addrinfo hints{}, * res, * p;
	hints.ai_family = AF_UNSPEC;      // IPv6 优先，不行再 IPv4
	hints.ai_socktype = SOCK_STREAM;

	std::string host_domain;
	std::cout << "Enter the server's domain or IP address,now you can enter zzkalinet.cn to connect with ShenJi." << std::endl;
	getline(std::cin, host_domain);

	if (getaddrinfo(host_domain.c_str(), std::to_string(PORT).c_str(), &hints, &res) != 0) {
		printf("DNS fail\n"); 
		return -1;
	}

	// 依次尝试每一个地址:contentReference[oaicite:5]{index=5}
	SOCKET client_socket = INVALID_SOCKET;
	for (p = res; p; p = p->ai_next) { 
		client_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (client_socket == INVALID_SOCKET) 
			continue;
		if (connect(client_socket, p->ai_addr, (int)p->ai_addrlen) == 0) 
			break;// 成功
#ifdef _WIN32
		closesocket(client_socket);
#else
		close(client_socket);
#endif
		client_socket = INVALID_SOCKET;
	}
	freeaddrinfo(res);

	if (client_socket == INVALID_SOCKET) { 
		err("Connection timeout!");
		return -1; 
	}
	std::cout << "Connect to server successfully." << std::endl;

	// 进入循环，不断从控制台输入消息并发送给服务器
	while (true) {
		string sending_str, control_msg,filename;
		getline(std::cin,sending_str);
		try {
			control_msg = sending_str.substr(0, 3);
			filename = sending_str.substr(3);
		}
		catch (std::out_of_range e) {
			std::cout << "Invalid argument." << std::endl;
			continue;
		}
		
		// 客户端希望接收文件信息
		if (control_msg == "-r ") {
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			char buffer[1];
			if (recvevery(client_socket, buffer, 1, 0) ==false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			if (buffer[0] == 'Y') {
				// 接收服务器的回显消息（文件大小）
				uint64_t NetSize = 0;
				if (recv(client_socket, reinterpret_cast<char*>(&NetSize), sizeof(NetSize), 0) <= 0) {
					std::cout << "server disconnect." << std::endl;
					break;
				}
				uint64_t FileSize = ntoh64(NetSize);
				std::cout << "File size: " << FileSize << " byte(s)" << std::endl;
				if (FileSize == 0) {
					std::cout << "Error! Server could not found the file. Or server refused to send an empty file." << std::endl;
					continue;
				}
				if (sendevery(client_socket, "OK\0", 3, 0) == false) {
					break;
				}
				char fp[BUF] = {};
				if (recvAll(client_socket, fp, FileSize, filename))
					std::cout << std::endl << "Receive file Successfully." << std::endl;
			}
			else if (buffer[0] == 'N') {
				std::cout << "Error! Server could not found the file." << std::endl;
			}
			else if (buffer[0] == 'E') {
				std::cout << "Error! Server refused to send an empty file." << std::endl;
			}
			else if (buffer[0] == 'L') {
				std::cout << "You need to log in first." << std::endl;
			}
			else {
				std::cout << "Received an unrecognized character." << std::endl;
			}
		}
		// 客户端希望发送文件信息
		else if (control_msg == "-s ") {
			FILE* fp = fopen(filename.c_str(), "rb");
			if (!fp) {
				std::cout << "File not found!" << std::endl;
				continue;
			}
			//跟文件尺寸相关的整数变量全部采用64位，防止溢出
#ifdef _WIN32
			_fseeki64(fp, 0, SEEK_END);
			uint64_t fsize = _ftelli64(fp);
			_fseeki64(fp, 0, SEEK_SET);
#else
			fseeko64(fp, 0, SEEK_END);
			uint64_t fsize = ftello64(fp);
			fseeko64(fp, 0, SEEK_SET);
#endif
			uint64_t netSize = hton64(fsize);
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			std::cout << "File size: " << fsize << " byte(s)" << std::endl;
			//发送文件大小
			if (sendevery(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0) == false) {
				std::cout << "Send FileSize error." << std::endl;
				break;
			}
			char a[1] = {};
			//接收服务器回传的确认信号（Y或者N）
			if (recvevery(client_socket, a, 1, 0) == false) {
				std::cout << "Failed to receive confirmation message from server." << std::endl;
				break;
			}
			//服务器确认可以发送
			if (a[0] == 'y' || a[0] == 'Y') {
				// 分块发送
				char buf[BUF];
				bool send_successful = true;
				if (!sendAll(client_socket, buf, fsize, fp)) {
					std::cout << std::endl << "Failed to send file." << std::endl;
					send_successful = false;
					break;
				}
				fclose(fp);
				if (send_successful)
					std::cout << std::endl << "File " << filename << " has been sended successfully." << std::endl;
			}
			//服务器拒绝接收文件
			else if(a[0] == 'n' || a[0] == 'N') {
				std::cout << "Server refused this file." << std::endl;
			}
			else if (a[0] == 'l' || a[0] == 'L') {
				std::cout << "You need to log in first." << std::endl;
				continue;
			}
		}
		// 客户端希望删除服务端的某个文件
		else if (control_msg == "-d ") {
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			char bac[128] = { '\0'};
			if (recv(client_socket, bac, sizeof(bac), 0) <= 0) {
				std::cout << "Failed to receive confirmation message from server." << std::endl;
				break;
			}
			std::cout << bac << std::endl;
		}
		else if (control_msg == "-c ") {
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			uint64_t netsize = 0;
			if (recvevery(client_socket, reinterpret_cast<char*>(&netsize), sizeof(netsize), 0) == false) {
				err("Failed to receive string length message.");
				break;
			}
			uint64_t len = ntoh64(netsize);
			if (len > 0) {
				std::string file_info(len, '\0');
				if (recvevery(client_socket, const_cast<char*>(file_info.c_str()), len, 0) == false) {
					err("Failed to receive file info.");
					break;
				}
				printf("%s", file_info.c_str());
			}
			else {
				std::cout << "The server storage is empty now." << std::endl;
			}
		}
		//客户端希望注册账号
		else if (control_msg == "-u ") {
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			char buffer_sql[DEFAULT_STR_LENGTH] = { 0 };
			if (recv(client_socket, buffer_sql, DEFAULT_STR_LENGTH, 0) <= 0) {
				std::cout << "Failed to receive ." << std::endl;
				break;
			}
			std::cout << buffer_sql << std::endl;
		}
		//客户端希望登录账号
		else if (control_msg == "-i ") {
			// 将输入的消息发送给服务器
			if (sendevery(client_socket, sending_str.c_str(), sending_str.size(), 0) == false) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			// 接收服务器的反馈
			char buffer_sql[DEFAULT_STR_LENGTH] = { 0 };
			if (recv(client_socket, buffer_sql, DEFAULT_STR_LENGTH, 0) <= 0) {
				std::cout << "Failed to receive." << std::endl;
				break;
			}
			std::cout << buffer_sql << std::endl;
		}
		else
			std::cout << "Invalid argument." << std::endl;
	}
	// 结束后关闭套接字
#ifdef _WIN32
	closesocket(client_socket);
#else
	close(client_socket);
#endif
	return 0;
}
