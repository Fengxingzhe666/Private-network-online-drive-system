//server.cpp               --模拟服务器程序
#include <iostream>        // 引入标准输入输出库
#include <cstring>         // 引入字符串处理函数，如 strlen 等
#include <WinSock2.h>      // Windows 平台的网络编程头文件
#include <thread>          // C++11 的多线程库，用于 std::thread
#pragma comment(lib, "ws2_32.lib") // 告诉编译器链接 ws2_32.lib（Windows Sockets 库）

using namespace std;       // 使用标准命名空间

int main(void)
{
    // 用于存储 WSAStartup 初始化信息的结构体
    WSADATA wsaData;
    // 初始化 Winsock，指定使用版本 2.2
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    // 创建服务器监听套接字，AF_INET=IPv4，SOCK_STREAM=TCP
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (listen_socket == INVALID_SOCKET) {
        // 若返回 INVALID_SOCKET 表示创建失败，打印错误，退出程序
        cerr << "error" << endl;
        return -1;
    }
    // 定义并初始化本地地址结构体
    struct sockaddr_in local = { 0 };
    // 协议族：IPv4
    local.sin_family = AF_INET;
    // 绑定到本机所有可用网卡(0.0.0.0)
    local.sin_addr.s_addr = INADDR_ANY;
    // 设置监听端口为 9999，htons 用于端口字节序转换
    local.sin_port = htons(9999);                         
    // 将监听套接字与本地地址结构体绑定
    if (bind(listen_socket, (struct sockaddr*)&local, sizeof local) == INVALID_SOCKET) {
        // 若返回 INVALID_SOCKET 表示绑定失败
        cerr << "error" << endl;                           
        return -1;
    }
    // 启动监听，第二个参数 backlog=128 表示排队等待连接的最大队列长度
    if (listen(listen_socket, 128) == -1) {
        // 如果 listen 失败，打印错误信息，退出程序
        cerr << "error" << endl;
        return -1;
    }
    std::cout << "Server starts,waitting for connection between client ..." << endl;
    // 进入循环，不断接受新的客户端连接
    while (true) {
        // 接受客户端连接
        SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
        // 检测是否接收成功
        if (client_socket == INVALID_SOCKET) {
            // 如果失败，则继续等待下一次连接
            continue;
        }
        // 打印新连接的套接字描述符
        std::cout << "client connect: " << client_socket << endl;
        // 创建一个线程来处理该客户端的收发数据
        thread th([](SOCKET client_socket) {
            while (true) {
                // 数据缓冲区，初始化为 0
                char buffer[1024] = { 0 };
                // 接收客户端数据
                int ret = recv(client_socket, buffer, sizeof buffer, 0);
                if (ret <= 0) {
                    // 若接收长度小于等于 0，表示客户端断开或出错
                    std::cout << "client disconnect: " << client_socket << endl;
                    // 退出循环，关闭连接
                    break;
                }
                // 在服务器端打印收到的消息
                std::cout << buffer << endl;
                // 将同样的消息回发给客户端（回显）
                send(client_socket, buffer, strlen(buffer), 0);
            }
            // 关闭与该客户端的连接
            closesocket(client_socket);
            // 将 client_socket 传入线程函数
            }, client_socket);
        // 将线程设为分离状态，让其独立运行，不用在主线程 join 等待
        th.detach();  
    }
    return 0; // 程序结束（在这个死循环逻辑里一般不会到达这里）
}

/*
————————————————
版权声明：本文为博主原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接和本声明。
原文链接：https ://blog.csdn.net/2301_82023822/article/details/144704929
*/
/*
代码说明
server.cpp：
使用 WSAStartup 初始化网络库，创建一个监听套接字并绑定到本地 9999 端口。
调用 listen() 开始监听，使用 accept() 阻塞等待客户端连接。
每当有客户端连接时，就创建一个新线程处理该客户端的消息收发，线程函数中使用 recv() 接收数据并回显给客户端。
线程结束后，关闭与该客户端的连接。
*/