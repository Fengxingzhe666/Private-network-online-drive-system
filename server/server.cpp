//server.cpp               --模拟服务器程序
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // 引入标准输入输出库
#include <cstring>         // 引入字符串处理函数，如 strlen 等
#include <WinSock2.h>      // Windows 平台的网络编程头文件
#include <mysql.h>
#include <thread>          // C++11 的多线程库，用于 std::thread
#include <vector>
#include <string>
#include <fstream>
#include <mswsock.h>
#include <stdio.h>
//#include <memory>          // 智能指针

#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib") // 告诉编译器链接 ws2_32.lib（Windows Sockets 库）
//#pragma comment(lib, "Mswsock.lib")// TransmitFile
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // 使用标准命名空间

constexpr int PORT = 5000;   //端口号
constexpr uint32_t SIZELIMIT = 1610612736;  //接收文件大小限制

const char* host = "127.0.0.1";
const char* user = "root";
const char* pw = "123456";
const char* databse_name = "zzk";
const std::string table_name = "account_password";
//const int MYSQL_PORT = 3306;

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
            std::string account_login;//已登录的账户名称，初始为空
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
                std::cout << buffer << std::endl;
                
                std::string control_msg(3, '\0');
                for (int i = 0;i < 3;++i)
                    control_msg[i] = buffer[i];
                if (control_msg == "-r ") {
                    if (account_login.empty()) {
                        if (send(client_socket, "L", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'L'." << std::endl;
                            break;
                        }
                        continue;
                    }
                    std::string path = "./files/" + account_login + "/";
                    for (int i = 3;i < ret;++i)
                        path.push_back(buffer[i]);
                    FILE* fp = fopen(path.c_str(), "rb");
                    if (!fp) {
                        std::cout << "File not found!" << std::endl;
                        if (send(client_socket, "N", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'N'." << std::endl;
                            break;
                        }
                        continue;
                    }
                    fseek(fp, 0, SEEK_END);
                    uint32_t fsize = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    uint32_t netSize = htonl(fsize);
                    if (fsize == 0) {
                        std::cout << "Empty file detected.Refuse to send back an empty file." << std::endl;
                        if (send(client_socket, "E", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'E'." << std::endl;
                            break;
                        }
                        continue;
                    }
                    if (send(client_socket, "Y", 1, 0) <= 0) {
                        std::cout << "Failed to send confirm message 'Y'." << std::endl;
                        break;
                    }
                    //发送8字节信息代表文件大小
                    if (send(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0) <= 0) {
                        std::cout << "Send FileSize error." << std::endl;
                        break;
                    }
                    char ok[3];
                    if (recv(client_socket, ok, 3, 0) <= 0) {
                        std::cout << "Failed to receive OK message." << std::endl;
                        break;
                    }
                    std::cout << ok << std::endl;
                    //分块发送
                    char buf[BUF];
                    //std::unique_ptr<char> buf(new char[BUF]);
                    size_t n;
                    bool send_successful = true;
                    while ((n = fread(buf, 1, sizeof buf, fp)) > 0) {
                        if (!sendAll(client_socket, buf, n)) {
                            std::cout << "Failed to send file." << std::endl;
                            send_successful = false;
                            break;
                        }
                    }
                    fclose(fp);
                    if (send_successful)
                        std::cout << std::endl << "File " << path << " has been sended successfully." << std::endl;
                }
                else if (control_msg == "-s ") {
                    // 接收客户端的消息（文件大小）
                    uint32_t NetSize = 0;
                    if (recv(client_socket, reinterpret_cast<char*>(&NetSize), sizeof(NetSize), 0) <= 0) {
                        std::cout << "Failed to receive FileSize." << std::endl;
                        break;
                    }
                    uint32_t FileSize = ntohl(NetSize);
                    if (account_login.empty()) {
                        if (send(client_socket, "L", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'L'." << std::endl;
                            break;
                        }
                        continue;
                    }
                    if (FileSize <= SIZELIMIT) {
                        if (send(client_socket, "Y", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'Y'." << std::endl;
                            break;
                        }
                    }
                    else {
                        if (send(client_socket, "N", 1, 0) <= 0) {
                            std::cout << "Failed to send confirm message 'N'." << std::endl;
                            break;
                        }
                    }
                    //路径转换
                    std::string filename, filename_pure;
                    for (int i = 4;i < ret;++i) {
                        filename.push_back(buffer[i]);
                    }
                    filename_pure = getfilename(filename);
                    //path = "./files/" + filename_pure;
                    char fp[BUF] = {};
                    //std::unique_ptr<char> fp(new char[BUF]);
                    if (recvAll(client_socket, fp, FileSize, filename_pure, account_login))
                        std::cout << std::endl << "Receive file Successfully." << std::endl;
                }
                // 客户端请求删除文件
                else if (control_msg == "-d ") {
                    std::string path = "./files/" + account_login + "/";
                    for (int i = 3;i < ret;++i)
                        path.push_back(buffer[i]);
                    std::string result;
                    if (account_login.empty()) {
                        result = "You need to log in first.";
                    }
                    else {
                        if (deleteFile(path))
                            result = "File " + path + "has been deleted successfully.";
                        else
                            result = "Fail to delete file";
                    }
                    if (send(client_socket, result.c_str(), 50, 0) <= 0) {
                        std::cout << "Failed to send confirm message." << std::endl;
                        break;
                    }
                }
                // 客户端请求注册账号
                else if (control_msg == "-u ") {
                    std::string account, password;
                    bool space = false;
                    for (int i = 3;i < ret;++i) {
                        if (buffer[i] == ' '){
                            space = true;
                            continue;
                        }
                        if (space)
                            password.push_back(buffer[i]);
                        else
                            account.push_back(buffer[i]);
                    }
                    //连接服务器本地的MySQL数据库
                    //初始化数据库
                    MYSQL* con = mysql_init(NULL);
                    //连接
                    if (!mysql_real_connect(con, host, user, pw, databse_name, MYSQL_PORT, NULL, 0)) {
                        fprintf_s(stderr, "Failed to connect to database. Error: %s\n", mysql_error(con));
                        send(client_socket, mysql_error(con), 100, 0);
                        break;
                    }
                    //设置连接的默认字符是 utf8，原始默认是 latin1，不使用可能会导致插入的中文是乱码
                    mysql_set_character_set(con, "utf-8");

                    std::string order = "insert into " + table_name + " values ('" + account + "','" + password + "');";
                    //执行命令
                    if (mysql_query(con, order.c_str())) {
                        //如果执行失败就打印
                        fprintf_s(stderr, "Error: %s\n", mysql_error(con));
                        send(client_socket, mysql_error(con), 100, 0);
                        continue;
                    }
                    std::string signup_succ = "Sign up successfully.";
                    if (send(client_socket, signup_succ.c_str(), signup_succ.size(), 0) <= 0) {
                        std::cout << "Connection failed." << std::endl;
                        break;
                    }
                    std::cout << signup_succ << std::endl;
                }
                // 客户端请求登录账号
                else if (control_msg == "-i ") {
                    std::string account, password;
                    bool space = false;
                    for (int i = 3;i < ret;++i) {
                        if (buffer[i] == ' ') {
                            space = true;
                            continue;
                        }
                        if (space)
                            password.push_back(buffer[i]);
                        else
                            account.push_back(buffer[i]);
                    }
                    //连接服务器本地的MySQL数据库
                    //初始化数据库
                    MYSQL* con = mysql_init(NULL);
                    //连接
                    if (!mysql_real_connect(con, host, user, pw, databse_name, MYSQL_PORT, NULL, 0)) {
                        fprintf_s(stderr, "Failed to connect to database. Error: %s\n", mysql_error(con));
                        send(client_socket, mysql_error(con), 100, 0);
                        break;
                    }
                    mysql_set_character_set(con, "utf-8");
                    std::string order = "select password from account_password where account='" + account + "';";
                    if (mysql_query(con, order.c_str())) {
                        //如果执行失败就打印
                        fprintf_s(stderr, "Error: %s\n", mysql_error(con));
                        if (send(client_socket, mysql_error(con), 100, 0) <= 0) {
                            std::cout << "Failed to send MySQL error code message." << std::endl;
                            break;
                        }
                        continue;
                    }
                    //保存查询结果
                    MYSQL_RES* res = mysql_store_result(con);
                    my_ulonglong row = mysql_num_rows(res);
                    unsigned int column = mysql_num_fields(res);
                    std::string answer;//该账号的密码
                    MYSQL_ROW line = mysql_fetch_row(res);
                    //指针为空说明账号不存在
                    if (!line) {
                        const std::string account_no = "The account you input does not exist.";
                        if (send(client_socket, account_no.c_str(), account_no.size(), 0) <= 0) {
                            std::cout << "Failed to send MySQL error code message." << std::endl;
                            break;
                        }
                    }
                    else {
                        answer += line[0];
                        std::string correct;
                        if (answer == password) {
                            correct = "Hello," + account + "!";
                            account_login = account;
                        }
                        else {
                            correct = "Incorrect password.";
                        }
                        if (send(client_socket, correct.c_str(), correct.size(), 0) <= 0) {
                            std::cout << "Failed to send MySQL error code message." << std::endl;
                            break;
                        }
                    }
                }
            }
            // 关闭与该客户端的连接
            closesocket(client_socket);
            account_login.clear();
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