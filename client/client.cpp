//client.cpp                         --ģ��ͻ���
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <memory>     //����ָ��
#include<Winsock2.h>
#include<WS2tcpip.h>
#include "../ProgressBar.h"
#include "../handleAll.h"
#pragma comment(lib, "ws2_32.lib")
#define err(errMsg)	std::cout<<errMsg<<"failed,code "<<WSAGetLastError()<<" line:"<<__LINE__<<std::endl;

using std::string;
constexpr int PORT = 5000;

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
		std::cerr << "Creat SOCKET error" << std::endl;
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
	std::cout << "Enter the server's ip address,such as 127.0.0.1 on this device itself." << std::endl;
	getline(std::cin, ip_str);
	// �����ı������򻷾���֧�� inet_pton������ inet_addr("127.0.0.1")
	inet_pton(AF_INET, ip_str.c_str(), &target.sin_addr.s_addr);
	//target.sin_addr.s_addr = inet_addr("127.0.0.1");

	// ���ӵ�����������ʧ�ܷ��� INVALID_SOCKET
	if (connect(client_socket, (struct sockaddr*)&target, sizeof target) == INVALID_SOCKET) {
		std::cerr << "Connection error, WSA " << WSAGetLastError() << std::endl;
		closesocket(client_socket);
		return -1;
	}
	std::cout << "client connects to server successfully." << std::endl;

	// ����ѭ�������ϴӿ���̨������Ϣ�����͸�������
	while (true) {
		string sending_str, control_msg,filename;
		getline(std::cin,sending_str);
		control_msg = sending_str.substr(0, 3);
		filename = sending_str.substr(3);
		// �ͻ���ϣ�������ļ���Ϣ
		if (control_msg == "-r ") {
			// ���������Ϣ���͸�������
			if (send(client_socket, sending_str.c_str(), sending_str.size(), 0) <= 0) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			// ���շ������Ļ�����Ϣ���ļ���С��
			uint32_t NetSize = 0;
			// �ӷ������������ݣ����������Ͽ������ʱ������ֵ <= 0
			if (recv(client_socket, reinterpret_cast<char*>(&NetSize), sizeof(NetSize), 0) <= 0) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			uint32_t FileSize = ntohl(NetSize);
			std::cout << "File size: " << FileSize << " byte(s)" << std::endl;
			if (FileSize == 0) {
				std::cout << "Error! Server could not found the file. Or server refused to send an empty file." << std::endl;
				continue;
			}
			send(client_socket, "OK\0", 3, 0);
			char fp[BUF] = {};
			//std::unique_ptr<char> fp(new char[BUF]);
			if (recvAll(client_socket, fp, FileSize, filename))
				std::cout << std::endl << "Receive file Successfully." << std::endl;
		}
		// �ͻ���ϣ�������ļ���Ϣ
		else if (control_msg == "-s ") {
			FILE* fp = fopen(filename.c_str(), "rb");
			if (!fp) {
				std::cout << "File not found!" << std::endl;
				continue;
			}
			fseek(fp, 0, SEEK_END);
			uint32_t fsize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			uint32_t netSize = htonl(fsize);
			// ���������Ϣ���͸�������
			if (send(client_socket, sending_str.c_str(), sending_str.size(), 0) <= 0) {
				std::cout << "server disconnect." << std::endl;
				break;
			}
			//�����ļ���С
			if (send(client_socket, reinterpret_cast<char*>(&netSize), sizeof(netSize), 0) <= 0) {
				std::cout << "Send FileSize error." << std::endl;
				break;
			}
			char a[1] = {};
			//���շ������ش���ȷ���źţ�Y����N��
			if (recv(client_socket, a, 1, 0) <= 0) {
				std::cout << "Failed to receive confirmation message from server." << std::endl;
				break;
			}
			//������ȷ�Ͽ��Է���
			if (a[0] == 'y' || a[0] == 'Y') {
				/*std::string filename_pure = getfilename(filename);
				if(send(client_socket,filename_pure.c_str(), filename_pure.size(),0) <= 0){
					std::cout << "Failed to send filename to server." << std::endl;
					break;
				}
				char ok[3];
				if (recv(client_socket, ok, 3, 0) <= 0) {
					std::cout << "Failed to receive OK message from server." << std::endl;
					break;
				}*/
				// �ֿ鷢��
				char buf[BUF];
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
					std::cout << std::endl << "File " << filename << " has been sended successfully." << std::endl;
			}
			//�������ܾ������ļ�
			else {
				std::cout << "Server refused this file." << std::endl;
			}
		}
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
