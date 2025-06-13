//client.cpp                         --模拟客户端
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<cstring>
#include <string>
#include<Winsock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;

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
    hints.ai_socktype = SOCK_DGRAM;   // 使用UDP协议
    hints.ai_protocol = IPPROTO_UDP;

    std::string host_domain;
    std::cout << "Enter the server's domain or IP address:" << std::endl;
    getline(std::cin, host_domain);

    if (getaddrinfo(host_domain.c_str(), std::to_string(PORT).c_str(), &hints, &res) != 0) {
        printf("DNS fail\n");
        return -1;
    }

    // 依次尝试每一个地址
    SOCKET client_socket = INVALID_SOCKET;
    for (p = res; p; p = p->ai_next) {
        client_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (client_socket == INVALID_SOCKET)
            continue;
        break;
    }

    freeaddrinfo(res);

    if (client_socket == INVALID_SOCKET) {
        err("Connection timeout!");
        return -1;
    }

    // 进入循环，不断从控制台输入消息并发送给服务器
    while (true) {
        // 用于存储用户输入的消息
        char buffer1[1024] = { 0 };
        cout << "Enter message: ";
        cin >> buffer1;
        // 将输入的消息发送给服务器
        sendto(client_socket, buffer1, strlen(buffer1), 0, p->ai_addr, (int)p->ai_addrlen);

        // 用于接收服务器的回显消息
        char buffer2[1024] = { 0 };
        // 从服务器接收数据
        int ret = recvfrom(client_socket, buffer2, sizeof(buffer2), 0, nullptr, nullptr);
        if (ret <= 0) {
            cout << "Server disconnected or error occurred." << endl;
            break;
        }
        // 打印服务器回显的消息
        cout << "Server response: " << buffer2 << endl;
    }

    // 结束后关闭套接字
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
/*
代码说明，client.cpp：
同样调用 WSAStartup 初始化网络库，创建一个套接字，使用 connect() 连接到服务器
在循环里不停地从用户输入获取字符串并发送给服务器，然后等待服务器的回显信息并打印。
*/
