//server.cpp               --ģ�����������
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // �����׼���������
#include <cstring>         // �����ַ������������� strlen ��
#include <WinSock2.h>      // Windows ƽ̨��������ͷ�ļ�
#include <thread>          // C++11 �Ķ��߳̿⣬���� std::thread
#include <vector>
#include <string>
#include <fstream>
#include <mswsock.h>
#include <memory>          // ����ָ��
#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib") // ���߱��������� ws2_32.lib��Windows Sockets �⣩
#pragma comment(lib, "Mswsock.lib")// TransmitFile
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // ʹ�ñ�׼�����ռ�

constexpr int PORT = 5000;   //�˿ں�

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
                std::cout << buffer << endl;
                //ifstream in("./data/DSC_1015-����ǿ-����.jpg");
                // ��ͬ������Ϣ�ط����ͻ��ˣ����ԣ�
                //send(client_socket, buffer, strlen(buffer), 0);

                
                std::string control_msg(3, '/0');
                for (int i = 0;i < 3;++i)
                    control_msg[i] = buffer[i];
                if (control_msg == "-r ") {
                    std::string path = "./files/";
                    for (int i = 3;i < ret;++i)
                        path.push_back(buffer[i]);
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
                    //����8�ֽ���Ϣ�����ļ���С
                    if (send(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0) <= 0) {
                        std::cout << "Send FileSize error." << std::endl;
                        break;
                    }
                    char ok[3];
                    recv(client_socket, ok, 3, 0);
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
                        std::cout << "File " << path << " has been sended successfully." << std::endl;
                }
            }
            // �ر���ÿͻ��˵�����
            closesocket(client_socket);
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