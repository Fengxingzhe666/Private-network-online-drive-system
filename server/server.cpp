//server.cpp               --模拟服务器程序
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // 引入标准输入输出库
#include <cstring>         // 引入字符串处理函数，如 strlen 等
#include <WinSock2.h>      // Windows 平台的网络编程头文件
#include <thread>          // C++11 的多线程库，用于 std::thread
#include <vector>
#include <string>
#include <fstream>
#include <mswsock.h>
#pragma comment(lib, "ws2_32.lib") // 告诉编译器链接 ws2_32.lib（Windows Sockets 库）
#pragma comment(lib, "Mswsock.lib")// TransmitFile
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // 使用标准命名空间

constexpr int PORT = 5000;
constexpr size_t BUF = 64 * 1024;          // 64 KiB

bool sendFile(SOCKET s, const char* fileName)
{
    FILE* read = fopen(fileName, "rb");
    if (!read)
    {
        perror("file open failed:\n");//输出描述性错误信息
        return false;
    }

    //获取文件大小
    fseek(read, 0, SEEK_END);	//将文件位置指针移动到最后
    long bufSize = ftell(read);	//ftell(FILE *stream)：返回给定流stream的当前文件位置，获取当前位置相对文件首的位移，位移值等于文件所含字节数
    fseek(read, 0, SEEK_SET);	//将文件位置指针移动到开头
    cout << "filesize:" << bufSize << endl;

    //把文件读到内存中来
    char* buffer = new char[bufSize];
    cout << sizeof(buffer) << endl;
    if (!buffer)
    {
        return false;
    }

    int nCount;
    int ret = 0;
    while ((nCount = fread(buffer, 1, bufSize, read)) > 0)	//循环读取文件进行传送
    {
        ret += send(s, buffer, nCount, 0);
        if (ret == SOCKET_ERROR)
        {
            err("sendFile");
            return false;
        }
    }
    shutdown(s, SD_SEND);
    recv(s, buffer, bufSize, 0);
    fclose(read);
    delete[] buffer;
    cout << "send file success!" << " Byte:" << ret << endl;
    //system("pause");
    return true;
}

bool sendAll(SOCKET s, const char* p, size_t len) {
    while (len) {
        int n = send(s, p, static_cast<int>(len), 0);
        if (n <= 0)
            return false;
        p += n; len -= n;
    }
    return true;
}

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
    // 设置监听端口，htons 用于端口字节序转换
    local.sin_port = htons(PORT);
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
                //ifstream in("./data/DSC_1015-已增强-降噪.jpg");
                // 将同样的消息回发给客户端（回显）
                //send(client_socket, buffer, strlen(buffer), 0);

                std::string path = "./files/";
                for (int i = 0;i < ret;++i) {
                    path.push_back(buffer[i]);
                }
                FILE* fp = fopen(path.c_str(), "rb");
                if (!fp) {
                    std::cout << "File not found!" << std::endl;
                    uint32_t fsize = 0;
                    send(client_socket, reinterpret_cast<char*>(&fsize), 4, 0);
                    continue;
                }

                fseek(fp, 0, SEEK_END);
                uint32_t fsize = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                uint32_t netSize = htonl(fsize);
                if (fsize == 0) {
                    std::cout << "Empty file detected.Refuse to send back an empty file." << std::endl;
                    send(client_socket, reinterpret_cast<char*>(&fsize), 4, 0);
                    continue;
                }
                //发送8字节信息代表文件大小
                if (send(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0)<=0) {
                    std::cout << "Send FileSize error." << std::endl;
                    break;
                }
                char ok[3];
                recv(client_socket, ok, 3, 0);
                std::cout << ok << std::endl;
                //分块发送
                char buf[BUF];
                size_t n;
                while ((n = fread(buf, 1, sizeof buf, fp)) > 0) {
                    if (!sendAll(client_socket, buf, n)) break;
                }
                fclose(fp);


                //HANDLE hFile = CreateFile(L"./files/screenshot.png", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                ////获取文件的大小
                //LARGE_INTEGER fileSize;
                //GetFileSizeEx(hFile, &fileSize);
                //size_t size_in_byte = fileSize.QuadPart;//文件的大小（单位：字节）
                //std::string size_str = std::to_string(size_in_byte);
                ////发送8字节信息代表文件大小
                //send(client_socket, size_str.c_str(), 8, 0);
                //char ok[3] = {};
                //recv(client_socket, ok, 3, 0);
                //std::cout << ok << std::endl;
                ////发送文件
                //bool file_successful = TransmitFile(client_socket, hFile, 0, 64000, NULL, NULL, 0);
                //if (file_successful)
                //    std::cout << "File sended successfully!" << std::endl;
                //else
                //    std::cout << "File sended failed!" << std::endl;
                
                //char* ok = new char[2];
                //recv(client_socket, ok, 1, 0);
                //if (ok == "OK") {
                //    delete[] ok;
                //    std::cout << "OK" << std::endl;
                //    break;
                //}
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
代码说明
server.cpp：
使用 WSAStartup 初始化网络库，创建一个监听套接字并绑定到本地端口。
调用 listen() 开始监听，使用 accept() 阻塞等待客户端连接。
每当有客户端连接时，就创建一个新线程处理该客户端的消息收发，线程函数中使用 recv() 接收数据并回显给客户端。
线程结束后，关闭与该客户端的连接。
*/