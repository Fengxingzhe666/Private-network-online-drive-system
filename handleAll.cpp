#include "handleAll.h"

// 接收文件函数，不包含账户名，由client端调用
bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename) {
	std::ofstream file("./files/" + filename, std::ios::out | std::ios::binary);
	size_t remain_byte = len;
	bool bar_continue = true;
	std::thread thbar([&]() {
		while (bar_continue) {
			showProgressBar(len - remain_byte, len);
			Sleep(100);
		}
		showProgressBar(len - remain_byte, len);
	});
	while (remain_byte > 0) {
		int n = recv(s, p, BUF, 0);
		if (n <= 0) {
			int wsaErr = WSAGetLastError();
			std::cerr << "Receive failed, code " << wsaErr << "\n";
			bar_continue = false;
			thbar.join();
			return false;
		}
		file.write(p, n);
		remain_byte -= n;
	}
	bar_continue = false;
	thbar.join();
	file.close();
	return true;
}
// 接收文件函数，包含账户名，由server端调用
bool recvAll(SOCKET s, char* p, size_t len, const std::string& filename, const std::string& account) {
	std::string path = "./files/" + account + "/" + filename;
	MkDir("./files/" + account);
	std::ofstream file(path, std::ios::out | std::ios::binary);
	size_t remain_byte = len;
	bool bar_continue = true;
	std::thread thbar([&]() {
		while (bar_continue) {
			showProgressBar(len - remain_byte, len);
			Sleep(100);
		}
		showProgressBar(len - remain_byte, len);
	});
	while (remain_byte > 0) {
		int n = recv(s, p, BUF, 0);
		if (n <= 0) {
			int wsaErr = WSAGetLastError();
			std::cerr << "Receive failed, code " << wsaErr << "\n";
			bar_continue = false;
			thbar.join();
			return false;
		}
		file.write(p, n);
		remain_byte -= n;
		//showProgressBar(len - remain_byte, len);
	}
	bar_continue = false;
	thbar.join();
	file.close();
	return true;
}

bool sendAll(SOCKET s, char* p, size_t len, FILE* stream) {
	size_t remain_byte = len;

	bool bar_continue = true;
	std::thread thbar([&]() {
		while (bar_continue) {
			showProgressBar(len - remain_byte, len);
			Sleep(100);
		}
		showProgressBar(len - remain_byte, len);
		});
	while (remain_byte) {
		int ret = fread(p, 1, BUF, stream);
		if (ret <= 0) {
			bar_continue = false;
			thbar.join();
			return false;
		}
		//循环发送ret字节
		int idx = 0;
		while (ret > 0) {
			int n = send(s, &p[idx], ret, 0);
			if (n <= 0){
				bar_continue = false;
				thbar.join();
				return false;
			}
			idx += n, ret -= n;
			remain_byte -= n;
		}
	}
	bar_continue = false;
	thbar.join();
	return true;
}
//从一长串路径字符串中提取文件名称（包含后缀）
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
// 删除文件
bool deleteFile(const std::string& filename) {
	int result = std::remove(filename.c_str());
	if (result == 0) {
		std::cout << "File " << filename << " has been deleted successfully." << std::endl;
		return true;
	}
	else {
		std::cerr << "Fail to delete file" << std::endl;
		return false;
	}
}