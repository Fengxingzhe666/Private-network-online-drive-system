//server.cpp               --ģ�����������
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>        // �����׼���������
#include <cstring>         // �����ַ������������� strlen ��
#include <WinSock2.h>      // Windows ƽ̨��������ͷ�ļ�
#include <WS2tcpip.h>
#include <thread>          // C++11 �Ķ��߳̿⣬���� std::thread
#include <vector>
#include <fstream>
#pragma comment(lib, "ws2_32.lib") // ���߱��������� ws2_32.lib��Windows Sockets �⣩
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // ʹ�ñ�׼�����ռ�

constexpr int PORT = 5000;

int main(void)
{
    // ���ڴ洢 WSAStartup ��ʼ����Ϣ�Ľṹ��
    WSADATA wsaData;
    // ��ʼ�� Winsock��ָ��ʹ�ð汾 2.2
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    // ���������������׽��֣�AF_INET=IPv4��SOCK_STREAM=TCP
    SOCKET listen_socket = socket(AF_INET6, SOCK_STREAM, 0); 
    if (listen_socket == INVALID_SOCKET) {
        // ������ INVALID_SOCKET ��ʾ����ʧ�ܣ���ӡ�����˳�����
        cerr << "error" << endl;
        return -1;
    }
    // ���岢��ʼ�����ص�ַ�ṹ��
    sockaddr_in6 local = { 0 };
    // Э���壺IPv4
    local.sin6_family = AF_INET6;
    // �󶨵��������п�������(0.0.0.0)
    local.sin6_addr = in6addr_any;
    // ���ü����˿ڣ�htons ���ڶ˿��ֽ���ת��
    local.sin6_port = htons(PORT);
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
                send(client_socket, buffer, strlen(buffer), 0);
                //sendFile(client_socket, "./DSC_1015-����ǿ-����.jpg");
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