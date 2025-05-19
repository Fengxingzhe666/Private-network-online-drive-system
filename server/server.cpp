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
#pragma comment(lib, "ws2_32.lib") // ���߱��������� ws2_32.lib��Windows Sockets �⣩
#pragma comment(lib, "Mswsock.lib")// TransmitFile
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;       // ʹ�ñ�׼�����ռ�

constexpr int PORT = 5000;
constexpr size_t BUF = 64 * 1024;          // 64 KiB

bool sendFile(SOCKET s, const char* fileName)
{
    FILE* read = fopen(fileName, "rb");
    if (!read)
    {
        perror("file open failed:\n");//��������Դ�����Ϣ
        return false;
    }

    //��ȡ�ļ���С
    fseek(read, 0, SEEK_END);	//���ļ�λ��ָ���ƶ������
    long bufSize = ftell(read);	//ftell(FILE *stream)�����ظ�����stream�ĵ�ǰ�ļ�λ�ã���ȡ��ǰλ������ļ��׵�λ�ƣ�λ��ֵ�����ļ������ֽ���
    fseek(read, 0, SEEK_SET);	//���ļ�λ��ָ���ƶ�����ͷ
    cout << "filesize:" << bufSize << endl;

    //���ļ������ڴ�����
    char* buffer = new char[bufSize];
    cout << sizeof(buffer) << endl;
    if (!buffer)
    {
        return false;
    }

    int nCount;
    int ret = 0;
    while ((nCount = fread(buffer, 1, bufSize, read)) > 0)	//ѭ����ȡ�ļ����д���
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
                //����8�ֽ���Ϣ�����ļ���С
                if (send(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0)<=0) {
                    std::cout << "Send FileSize error." << std::endl;
                    break;
                }
                char ok[3];
                recv(client_socket, ok, 3, 0);
                std::cout << ok << std::endl;
                //�ֿ鷢��
                char buf[BUF];
                size_t n;
                while ((n = fread(buf, 1, sizeof buf, fp)) > 0) {
                    if (!sendAll(client_socket, buf, n)) break;
                }
                fclose(fp);


                //HANDLE hFile = CreateFile(L"./files/screenshot.png", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                ////��ȡ�ļ��Ĵ�С
                //LARGE_INTEGER fileSize;
                //GetFileSizeEx(hFile, &fileSize);
                //size_t size_in_byte = fileSize.QuadPart;//�ļ��Ĵ�С����λ���ֽڣ�
                //std::string size_str = std::to_string(size_in_byte);
                ////����8�ֽ���Ϣ�����ļ���С
                //send(client_socket, size_str.c_str(), 8, 0);
                //char ok[3] = {};
                //recv(client_socket, ok, 3, 0);
                //std::cout << ok << std::endl;
                ////�����ļ�
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