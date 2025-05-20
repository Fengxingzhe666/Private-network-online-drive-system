#include "handleAll.h"

bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename) {
	std::ofstream file("./files/" + filename, std::ios::out | std::ios::binary);
	size_t remain_byte = len;
	while (remain_byte > 0) {
		int n = recv(s, p, BUF, 0);
		if (n == -1) {
			int wsaErr = WSAGetLastError();
			std::cerr << "Receive failed, code " << wsaErr << "\n";
		}
		if (n <= 0)
			return false;
		file.write(p, n);
		remain_byte -= n;
		showProgressBar(len - remain_byte, len);
	}
	file.close();
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