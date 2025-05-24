//client.cpp                         --模拟客户端
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<cstring>
#include<Winsock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;

constexpr int PORT = 50000;

int main()
{
	// 存储 WSAStartup 初始化信息的结构体
	WSADATA wsaData;
	// 初始化 Winsock，指定使用版本 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建客户端套接字，AF_INET=IPv4，SOCK_STREAM=TCP
	SOCKET client_socket = socket(AF_INET6, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		// 若创建失败，打印错误并退出
		cerr << "Creat SOCKET error" << endl;
		return -1;
	}

	// 存储服务器信息的地址结构
	struct sockaddr_in6 target = { 0 };
	// 协议族：IPv4
	target.sin6_family = AF_INET6;
	// 目标端口号（要与服务器保持一致）
	target.sin6_port = htons(PORT);
	// inet_pton: 将字符串形式的 IP("127.0.0.1") 转为网络字节序地址并存储到 sin_addr.s_addr
	// 如果你的编译器或环境不支持 inet_pton，可用 inet_addr("127.0.0.1")
	inet_pton(AF_INET6, "::1", &target.sin6_addr);
	//target.sin_addr.s_addr = inet_addr("127.0.0.1");

	// 连接到服务器，若失败返回 INVALID_SOCKET
	if (connect(client_socket, (struct sockaddr*)&target, sizeof target) == INVALID_SOCKET) {
		std::cerr << "Connection error, WSA " << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		return -1;
	}
	cout << "client connects to server successfully." << endl;

	// 进入循环，不断从控制台输入消息并发送给服务器
	while (true) {
		// 用于存储用户输入的消息
		char buffer1[1024] = { 0 };
		cout << "enter: ";
		cin >> buffer1;
		// 将输入的消息发送给服务器
		send(client_socket, buffer1, strlen(buffer1), 0);

		// 用于接收服务器的回显消息
		char buffer2[1024] = { 0 };
		// 从服务器接收数据
		int ret = recv(client_socket, buffer2, sizeof buffer2, 0);
		// 当服务器断开或出错时，返回值 <= 0
		if (ret <= 0) {
			cout << "server disconnect." << endl;
		}
		// 打印服务器回显的消息
		cout << buffer2 << endl;

		//recvFile(client_socket, "./files/1.txt");

	}
	// 结束后关闭套接字
	closesocket(client_socket);
	return 0;
}
/*
	————————————————
		版权声明：本文为博主原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接和本声明。
		原文链接：https ://blog.csdn.net/2301_82023822/article/details/144704929
*/
/*
代码说明，client.cpp：
同样调用 WSAStartup 初始化网络库，创建一个套接字，使用 connect() 连接到服务器的 127.0.0.1:9999
在循环里不停地从用户输入获取字符串并发送给服务器，然后等待服务器的回显信息并打印。
*/
