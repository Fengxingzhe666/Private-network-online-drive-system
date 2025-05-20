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
	size_t remain_byte = len;
	while (remain_byte) {
		int n = send(s, p, static_cast<int>(len), 0);
		if (n <= 0)
			return false;
		p += n; remain_byte -= n;
		showProgressBar(len - remain_byte, len);
	}
	return true;
}

std::string getfilename(const std::string& str) {
	std::string filename;
	int last = 0;
	for (int i = str.size() - 1;i >= 0;--i) {
		if (str[i] == '/') {
			last = i + 1;
			break;
		}
	}
	filename.insert(filename.begin(), str.begin() + last, str.end());
	return filename;
}