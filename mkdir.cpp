#include "mkdir.h"

void MkDir(std::string folder)
{
	if (0 == ACCESS(folder.c_str(), 0)) //判断路径是否存在，如果存在不需要再创建了
		return;
	PATH_FIX(folder);                     // 仅 Windows 需要 将路径中的 正斜杠 统一替换成 反斜杠
	std::string folder_builder; //子文件夹路径，包含上一级的路径
	std::string sub;		//要检测的子文件夹名字
	sub.reserve(folder.size());
	for (auto it = folder.begin(); it != folder.end(); ++it) //遍历路径
	{
		const char c = *it;
		sub.push_back(c);
		if (c == '\\' || it == folder.end() - 1) //如果遇到反斜杠 或者 结尾了，就可以判断是否存在了，不存在时要创建
		{
			folder_builder.append(sub);//上一级路径 + 现在的文件夹名称
			if (0 != ACCESS(folder_builder.c_str(), 0)) //检查现在的文件夹是否存在
			{
				if (0 != MKDIR(folder_builder.c_str())) //不存在时需要创建
					return;//创建失败
			}
			sub.clear();//清空文件夹名称，然后才能存下一级的文件夹名称
		}
	}
	// 该函数原文链接：https ://blog.csdn.net/hss2799/article/details/131529800
}

#ifdef _WIN32
void getFiles(std::string path, vector<string>& files)
{
    //文件句柄  
	long long hFile = 0;//这里将原文long修改为long long，否则会报错
    //文件信息，声明一个存储文件信息的结构体  
    struct _finddata_t fileinfo;
    string p;//字符串，存放路径
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)//若查找成功则入
    {
        do
        {
            //如果是目录,迭代之（即文件夹内还有文件夹）  
            if ((fileinfo.attrib &  _A_SUBDIR))
            {
                //文件名不等于"."&&文件名不等于".."
                //.表示当前目录
                //..表示当前目录的父目录
                //判断时，两者都要忽略，不然就无限递归跳不出去了！
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
            }
            //如果不是,加入列表  
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);
        //_findclose函数结束查找
        _findclose(hFile);
    }
	// 该函数原文链接：https://blog.csdn.net/a8039974/article/details/87930602
}
#else
void getFiles(string path, vector<string>& files)
{
    DIR* dir = opendir(path.c_str());          // 打开目录
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)  // 逐项读取
    {
        // 跳过 "." 和 ".."
        if (!std::strcmp(entry->d_name, ".") || !std::strcmp(entry->d_name, ".."))
            continue;

        std::string full = path + "/" + entry->d_name;   // 拼绝对路径
        struct stat st {};
        if (stat(full.c_str(), &st) == -1)               // 取元数据
            continue;

        if (S_ISDIR(st.st_mode))                         // 是目录则递归
        {
            getFiles(full, files);
        }
        else                                             // 否则收集文件
        {
            files.push_back(full);
        }
    }
    closedir(dir);                                       // 关闭目录流
}
#endif