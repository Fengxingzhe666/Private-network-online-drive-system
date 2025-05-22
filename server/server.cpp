//server.cpp               --ģ�����������
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // �����׼���������
#include <cstring>         // �����ַ������������� strlen ��
#include <WinSock2.h>      // Windows ƽ̨��������ͷ�ļ�
#include <mysql.h>
#include <thread>          // C++11 �Ķ��߳̿⣬���� std::thread
#include <vector>
#include <string>
#include <fstream>
#include <mswsock.h>
#include <stdio.h>
//#include <memory>          // ����ָ��

#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib") // ���߱��������� ws2_32.lib��Windows Sockets �⣩
//#pragma comment(lib, "Mswsock.lib")// TransmitFile
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // ʹ�ñ�׼�����ռ�

constexpr int PORT = 5000;   //�˿ں�
constexpr uint32_t SIZELIMIT = 1610612736;  //�����ļ���С����

const char* host = "127.0.0.1";
const char* user = "root";
const char* pw = "123456";
const char* databse_name = "zzk";
const std::string table_name = "account_password";
//const int MYSQL_PORT = 3306;

int main(void)
{
    // ���ڴ洢 WSAStartup ��ʼ����Ϣ�Ľṹ��
    WSADATA wsaData;
    // ��ʼ�� Winsock��ָ��ʹ�ð汾 2.2
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    // ���������������׽��֣�AF_INET=IPv4��SOCK_STREAM=TCP
    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (listen_socket == INVALID_SOCKET) {
        // ������ INVALID_SOCKET ��ʾ����ʧ�ܣ���ӡ�����˳�����
        cerr << "error" << endl;
        return -1;
    }
    // ���岢��ʼ�����ص�ַ�ṹ��
    struct sockaddr_in local = { 0 };
    // Э���壺IPv4
    local.sin_family = AF_INET;
    // �󶨵��������п�������(0.0.0.0)
    local.sin_addr.s_addr = INADDR_ANY;
    // ���ü����˿ڣ�htons ���ڶ˿��ֽ���ת��
    local.sin_port = htons(PORT);
    // �������׽����뱾�ص�ַ�ṹ���
    if (bind(listen_socket, (struct sockaddr*)&local, sizeof local) == INVALID_SOCKET) {
        // ������ INVALID_SOCKET ��ʾ��ʧ��
        cerr << "error" << endl;
        return -1;
    }
    // �����������ڶ������� backlog=128 ��ʾ�Ŷӵȴ����ӵ������г���
    if (listen(listen_socket, 128) == -1) {
        // ��� listen ʧ�ܣ���ӡ������Ϣ���˳�����
        cerr << "error" << endl;
        return -1;
    }
    std::cout << "Server starts,waitting for connection between client ..." << endl;
    // ����ѭ�������Ͻ����µĿͻ�������
    while (true) {
        // ���ܿͻ�������
        SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
        // ����Ƿ���ճɹ�
        if (client_socket == INVALID_SOCKET) {
            // ���ʧ�ܣ�������ȴ���һ������
            continue;
        }
        // ��ӡ�����ӵ��׽���������
        std::cout << "client connect: " << client_socket << endl;
        // ����һ���߳�������ÿͻ��˵��շ�����
        thread th([](SOCKET client_socket) {
            std::string account_login;//�ѵ�¼���˻����ƣ���ʼΪ��
            while (true) {
                // ���ݻ���������ʼ��Ϊ 0
                char buffer[1024] = { 0 };
                // ���տͻ�������
                int ret = recv(client_socket, buffer, sizeof buffer, 0);
                if (ret <= 0) {
                    // �����ճ���С�ڵ��� 0����ʾ�ͻ��˶Ͽ������
                    std::cout << "client disconnect: " << client_socket << endl;
                    // �˳�ѭ�����ر�����
                    break;
                }
                // �ڷ������˴�ӡ�յ�����Ϣ
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
                    //����8�ֽ���Ϣ�����ļ���С
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
                    //�ֿ鷢��
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
                    // ���տͻ��˵���Ϣ���ļ���С��
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
                    //·��ת��
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
                // �ͻ�������ɾ���ļ�
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
                // �ͻ�������ע���˺�
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
                    //���ӷ��������ص�MySQL���ݿ�
                    //��ʼ�����ݿ�
                    MYSQL* con = mysql_init(NULL);
                    //����
                    if (!mysql_real_connect(con, host, user, pw, databse_name, MYSQL_PORT, NULL, 0)) {
                        fprintf_s(stderr, "Failed to connect to database. Error: %s\n", mysql_error(con));
                        send(client_socket, mysql_error(con), 100, 0);
                        break;
                    }
                    //�������ӵ�Ĭ���ַ��� utf8��ԭʼĬ���� latin1����ʹ�ÿ��ܻᵼ�²��������������
                    mysql_set_character_set(con, "utf-8");

                    std::string order = "insert into " + table_name + " values ('" + account + "','" + password + "');";
                    //ִ������
                    if (mysql_query(con, order.c_str())) {
                        //���ִ��ʧ�ܾʹ�ӡ
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
                // �ͻ��������¼�˺�
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
                    //���ӷ��������ص�MySQL���ݿ�
                    //��ʼ�����ݿ�
                    MYSQL* con = mysql_init(NULL);
                    //����
                    if (!mysql_real_connect(con, host, user, pw, databse_name, MYSQL_PORT, NULL, 0)) {
                        fprintf_s(stderr, "Failed to connect to database. Error: %s\n", mysql_error(con));
                        send(client_socket, mysql_error(con), 100, 0);
                        break;
                    }
                    mysql_set_character_set(con, "utf-8");
                    std::string order = "select password from account_password where account='" + account + "';";
                    if (mysql_query(con, order.c_str())) {
                        //���ִ��ʧ�ܾʹ�ӡ
                        fprintf_s(stderr, "Error: %s\n", mysql_error(con));
                        if (send(client_socket, mysql_error(con), 100, 0) <= 0) {
                            std::cout << "Failed to send MySQL error code message." << std::endl;
                            break;
                        }
                        continue;
                    }
                    //�����ѯ���
                    MYSQL_RES* res = mysql_store_result(con);
                    my_ulonglong row = mysql_num_rows(res);
                    unsigned int column = mysql_num_fields(res);
                    std::string answer;//���˺ŵ�����
                    MYSQL_ROW line = mysql_fetch_row(res);
                    //ָ��Ϊ��˵���˺Ų�����
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
            // �ر���ÿͻ��˵�����
            closesocket(client_socket);
            account_login.clear();
            // �� client_socket �����̺߳���
            }, client_socket);
        // ���߳���Ϊ����״̬������������У����������߳� join �ȴ�
        th.detach();  
    }
    return 0; // ����������������ѭ���߼���һ�㲻�ᵽ�����
}
/*
����˵��
server.cpp��
ʹ�� WSAStartup ��ʼ������⣬����һ�������׽��ֲ��󶨵����ض˿ڡ�
���� listen() ��ʼ������ʹ�� accept() �����ȴ��ͻ������ӡ�
ÿ���пͻ�������ʱ���ʹ���һ�����̴߳���ÿͻ��˵���Ϣ�շ����̺߳�����ʹ�� recv() �������ݲ����Ը��ͻ��ˡ�
�߳̽����󣬹ر���ÿͻ��˵����ӡ�
*/