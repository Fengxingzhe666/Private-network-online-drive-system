//client.cpp                         --ģ��ͻ���
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include<cstring>
#include <string>
#include<Winsock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;

constexpr int PORT = 5000;
constexpr size_t BUF = 64 * 1024;          // 64 KiB

bool recvAll(SOCKET s, char* p, size_t len,const string& filename) {
	std::ofstream file("./files/" + filename, std::ios::out | std::ios::binary);
	while (len > 0) {
		int n = recv(s, p, BUF, 0);
		if (n == -1) {
			int wsaErr = WSAGetLastError();
			std::cerr << "Receive failed, code " << wsaErr << "\n";
		}
		if (n <= 0)
			return false;
		file.write(p, n);
		len -= n;
	}
	file.close();
	return true;
}

int main()
{
	// �洢 WSAStartup ��ʼ����Ϣ�Ľṹ��
	WSADATA wsaData;
	// ��ʼ�� Winsock��ָ��ʹ�ð汾 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// �����ͻ����׽��֣�AF_INET=IPv4��SOCK_STREAM=TCP
	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket == INVALID_SOCKET) {
		// ������ʧ�ܣ���ӡ�����˳�
		cerr << "Creat SOCKET error" << endl;
		return -1;
	}

	// �洢��������Ϣ�ĵ�ַ�ṹ
	struct sockaddr_in target;
	// Э���壺IPv4
	target.sin_family = AF_INET;
	// Ŀ��˿ںţ�Ҫ�����������һ�£�
	target.sin_port = htons(PORT);
	// inet_pton: ���ַ�����ʽ�� IP("127.0.0.1") תΪ�����ֽ����ַ���洢�� sin_addr.s_addr
	string ip_str;
	cout << "Enter the server's ip address,such as 127.0.0.1 on this device itself." << std::endl;
	cin >> ip_str;
	// �����ı������򻷾���֧�� inet_pton������ inet_addr("127.0.0.1")
	inet_pton(AF_INET, ip_str.c_str(), &target.sin_addr.s_addr);
	//target.sin_addr.s_addr = inet_addr("127.0.0.1");

	// ���ӵ�����������ʧ�ܷ��� INVALID_SOCKET
	if (connect(client_socket, (struct sockaddr*)&target, sizeof target) == INVALID_SOCKET) {
		std::cerr << "Connection error, WSA " << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		return -1;
	}
	cout << "client connects to server successfully." << endl;

	// ����ѭ�������ϴӿ���̨������Ϣ�����͸�������
	while (true) {
		// ���ڴ洢�û��������Ϣ
		//char buffer1[1024] = { 0 };
		string filename;
		cout << "Enter file name: ";
		cin >> filename;
		// ���������Ϣ���͸�������
		if (send(client_socket, filename.c_str(), filename.size(), 0) <= 0) {
			cout << "server disconnect." << endl;
			break;
		}

		// ���ڽ��շ������Ļ�����Ϣ
		char buffer2[8] = { 0 };
		uint32_t NetSize = 0;
		// �ӷ������������ݣ����������Ͽ������ʱ������ֵ <= 0
		if (recv(client_socket, reinterpret_cast<char*>(&NetSize), sizeof(NetSize), 0) <= 0) {
			cout << "server disconnect." << endl;
			break;
		}

		uint32_t FileSize = ntohl(NetSize);
		cout << "File size: " << FileSize <<" byte(s)" << endl;
		if (FileSize == 0) {
			cout << "Error! Server could not found the file. Or server refused to send an empty file." << endl;
			continue;
		}
		send(client_socket, "OK\0", 3, 0);
		char fp[BUF] = {};
		if (recvAll(client_socket, fp, FileSize,filename))
			std::cout << "Receive file Successfully." << std::endl;
	}
	// ������ر��׽���
	closesocket(client_socket);
	return 0;
}
/*
����˵����client.cpp��
ͬ������ WSAStartup ��ʼ������⣬����һ���׽��֣�ʹ�� connect() ���ӵ��������� 127.0.0.1:9999
��ѭ���ﲻͣ�ش��û������ȡ�ַ��������͸���������Ȼ��ȴ��������Ļ�����Ϣ����ӡ��
*/
