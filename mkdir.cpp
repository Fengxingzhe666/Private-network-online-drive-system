#include "mkdir.h"

void MkDir(std::string folder)
{
	if (0 == ::_access(folder.c_str(), 0)) //�ж�·���Ƿ���ڣ�������ڲ���Ҫ�ٴ�����
	{
		return;
	}
	std::replace(folder.begin(), folder.end(), '/', '\\');	 //��·���е� ��б�� ͳһ�滻�� ��б��
	std::string folder_builder; //���ļ���·����������һ����·��
	std::string sub;		//Ҫ�������ļ�������
	sub.reserve(folder.size());
	for (auto it = folder.begin(); it != folder.end(); ++it) //����·��
	{
		const char c = *it;
		sub.push_back(c);
		if (c == '\\' || it == folder.end() - 1) //���������б�� ���� ��β�ˣ��Ϳ����ж��Ƿ�����ˣ�������ʱҪ����
		{
			folder_builder.append(sub);//��һ��·�� + ���ڵ��ļ�������
			if (0 != ::_access(folder_builder.c_str(), 0)) //������ڵ��ļ����Ƿ����
			{
				if (0 != ::_mkdir(folder_builder.c_str())) //������ʱ��Ҫ����
				{
					return;//����ʧ��
				}
				else
				{
					//�����ļ��гɹ���
				}
			}
			sub.clear();//����ļ������ƣ�Ȼ����ܴ���һ�����ļ�������
		}
	}
}
/*
��������������������������������
��Ȩ����������Ϊ����ԭ�����£���ѭ CC 4.0 BY - SA ��ȨЭ�飬ת���븽��ԭ�ĳ������Ӻͱ�������
ԭ�����ӣ�https ://blog.csdn.net/hss2799/article/details/131529800
*/