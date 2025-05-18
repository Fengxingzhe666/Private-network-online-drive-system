//client.cpp                         --ģ��ͻ���
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include<cstring>
#include<Winsock2.h>
#include<WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<endl;

using namespace std;

constexpr int PORT = 5000;
long bufSize = 10 * 1024;	//��������С
char* buffer;
bool recvFile(SOCKET s, const char* fileName)
{
	if (buffer == NULL)
	{
		buffer = new char[bufSize];
		if (!buffer)
			return false;
	}
	//	�������ļ�
	FILE* write = fopen(fileName, "wb");
	if (!write)
	{
		perror("file write failed:\n");
		return false;
	}

	int ret = 0;
	int nCount;
	while ((nCount = recv(s, buffer, bufSize, 0)) > 0)	//ѭ�������ļ�������
	{
		ret += fwrite(buffer, nCount, 1, write);
	}
	if (ret == 0)
	{
		cout << "server offline" << endl;
	}
	else if (ret < 0)
	{
		err("recv");
		return false;
	}
	cout << "receive file success!" << endl;

	fclose(write);
	cout << "save file success! Filename:" << fileName << endl;
	//system("pause");
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
	// �����ı������򻷾���֧�� inet_pton������ inet_addr("127.0.0.1")
	inet_pton(AF_INET, "10.192.137.27", &target.sin_addr.s_addr);
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
		char buffer1[1024] = { 0 };
		cout << "enter: ";
		cin >> buffer1;
		// ���������Ϣ���͸�������
		send(client_socket, buffer1, strlen(buffer1), 0);

		// ���ڽ��շ������Ļ�����Ϣ
		char buffer2[1024] = { 0 };
		// �ӷ�������������
		int ret = recv(client_socket, buffer2, sizeof buffer2, 0);
		// ���������Ͽ������ʱ������ֵ <= 0
		if (ret <= 0) {
			cout << "server disconnect." << endl;
		}
		// ��ӡ���������Ե���Ϣ
		cout << buffer2 << endl;

		//recvFile(client_socket, "./files/1.txt");

	}
	// ������ر��׽���
	closesocket(client_socket);
	return 0;
}
/*
	��������������������������������
		��Ȩ����������Ϊ����ԭ�����£���ѭ CC 4.0 BY - SA ��ȨЭ�飬ת���븽��ԭ�ĳ������Ӻͱ�������
		ԭ�����ӣ�https ://blog.csdn.net/2301_82023822/article/details/144704929
*/
/*
����˵����client.cpp��
ͬ������ WSAStartup ��ʼ������⣬����һ���׽��֣�ʹ�� connect() ���ӵ��������� 127.0.0.1:9999
��ѭ���ﲻͣ�ش��û������ȡ�ַ��������͸���������Ȼ��ȴ��������Ļ�����Ϣ����ӡ��
*/
