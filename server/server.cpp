//server.cpp               --模拟服务器程序
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // 引入标准输入输出库
#include <cstring>         // 引入字符串处理函数，如 strlen 等
#include <WinSock2.h>      // Windows 平台的网络编程头文件
#include <WS2tcpip.h>
#include <thread>          // C++11 的多线程库，用于 std::thread
#include <vector>
#include <fstream>
#pragma comment(lib, "ws2_32.lib") // 告诉编译器链接 ws2_32.lib（Windows Sockets 库）
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // 使用标准命名空间

constexpr int PORT = 5000;

int main(void)
{
    // 用于存储 WSAStartup 初始化信息的结构体
    WSADATA wsaData;
    // 初始化 Winsock，指定使用版本 2.2
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    // 创建服务器监听套接字，SOCK_DGRAM 为UDP协议
    SOCKET server_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET) {
        // 若返回 INVALID_SOCKET 表示创建失败，打印错误，退出程序
        cerr << "error" << endl;
        return -1;
    }

    // 定义并初始化本地地址结构体
    sockaddr_in6 local = { 0 };
    // 协议族：IPv6
    local.sin6_family = AF_INET6;
    // 绑定到本机所有可用网卡(0.0.0.0)
    local.sin6_addr = in6addr_any;
    // 设置监听端口，htons 用于端口字节序转换
    local.sin6_port = htons(PORT);
    // 兼容ipv4/6
    int zero = 0;
    if (setsockopt(server_socket, IPPROTO_IPV6, IPV6_V6ONLY,
        reinterpret_cast<char*>(&zero), sizeof(zero)) == SOCKET_ERROR) {
        err("setsockopt IPV6_V6ONLY"); 
        return -1;
    }

    // 将服务器套接字与本地地址结构体绑定
    if (bind(server_socket, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        cerr << "error" << endl;
        return -1;
    }
    std::cout << "UDP Echo-Server started, port " << PORT << std::endl;

    // 进入循环，不断接收客户端发送的消息
    while (true) {
        sockaddr_in6 client_addr;
        int addr_len = sizeof(client_addr);
        char buffer[1024] = { 0 };

        // 接收客户端消息
        int ret = recvfrom(server_socket, buffer, sizeof(buffer), 0, (sockaddr*)&client_addr, &addr_len);
        if (ret <= 0) {
            // 若接收长度小于等于 0，表示出错
            std::cout << "Error receiving message" << std::endl;
            continue;
        }
        std::cout << "Received message: " << buffer << std::endl;

        // 回显接收到的消息
        sendto(server_socket, buffer, strlen(buffer), 0, (sockaddr*)&client_addr, addr_len);
    }

    // 关闭套接字
    closesocket(server_socket);
    WSACleanup();
    return 0;
}
/*
代码说明
server.cpp：
使用 WSAStartup 初始化网络库，创建一个监听套接字并绑定到本地端口。
调用 listen() 开始监听，使用 accept() 阻塞等待客户端连接。
每当有客户端连接时，就创建一个新线程处理该客户端的消息收发，线程函数中使用 recv() 接收数据并回显给客户端。
线程结束后，关闭与该客户端的连接。
*/