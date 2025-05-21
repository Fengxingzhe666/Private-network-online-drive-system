#include "mkdir.h"

void MkDir(std::string folder)
{
	if (0 == ::_access(folder.c_str(), 0)) //判断路径是否存在，如果存在不需要再创建了
	{
		return;
	}
	std::replace(folder.begin(), folder.end(), '/', '\\');	 //将路径中的 正斜杠 统一替换成 反斜杠
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
			if (0 != ::_access(folder_builder.c_str(), 0)) //检查现在的文件夹是否存在
			{
				if (0 != ::_mkdir(folder_builder.c_str())) //不存在时需要创建
				{
					return;//创建失败
				}
				else
				{
					//创建文件夹成功了
				}
			}
			sub.clear();//清空文件夹名称，然后才能存下一级的文件夹名称
		}
	}
}
/*
————————————————
版权声明：本文为博主原创文章，遵循 CC 4.0 BY - SA 版权协议，转载请附上原文出处链接和本声明。
原文链接：https ://blog.csdn.net/hss2799/article/details/131529800
*/