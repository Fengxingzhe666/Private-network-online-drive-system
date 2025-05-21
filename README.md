# 模拟私有云盘系统  
**TCP**  **SOCKET编程**  **应用层文件传输协议**  **MySQL**  
如果服务器端的路由器有公网ipv4，就可以直接使用其公网ipv4，但一般情况现在ISP都不会给免费的公网ipv4，这种情况下由于上层还存在更复杂的NAT，客户端直接用服务器端浏览器显示的公网ip是无法ping通的。一种方案是付费租用云服务器，将server.cpp部署在云服务器。另一种方案是搭建VPN网络点对点进行通信。  
## 搭建点对点的VPN网络（内网穿透）  
使用工具Tailscale，下载地址：https://tailscale.com/download/windows  
在服务器端和客户端分别下载Tailscale，并登录同一个账号。先在服务器端编译运行server.cpp，再在客户端编译运行client.cpp（顺序不能反）（编译可以用自己的编译器例如Visual studio 2022或者g++）。客户端运行client.cpp以后要先输入服务器端Tailscale给的ipv4地址，如果服务器端和客户端都有网络连接且登录的同一个Tailscale账号，此时应该能看到英文的连接成功提示  
```
Enter the server's ip address,such as 127.0.0.1 on this device itself.
10.x.y.z # 这里输入服务器端的Tailscale分配的ipv4地址或者路由的有效公网ipv4地址
client connects to server successfully. # 提示连接成功
```
## 客户端命令  
客户端命令由控制信息+文件名信息组成。  
控制信息包括：-r从服务器端接收文件信息；-s向服务器发送文件信息；-d删除服务器端的文件；-i登录账号；-u注册账号：  
目前已完成的命令：  
```
-r filename.type # 从服务器端接收文件filename.type，其中type是文件后缀
-s filename.type # 将本地文件filename.type上传到服务器，filename.type之前必须包含该文件在客户端本地的路径
-d filename.type # 删除服务端的文件
-u account password # 向服务器请求注册账号，后边跟着密码，注意账号密码中不允许有空格，账号和密码的长度不超过20
-i account password # 向服务器请求登录账号，后边跟着密码
```
**在进行其他操作前，必须使用-i命令登录一个账号**  
注意因为使用-u和-i功能，在服务器端必须安装MySQL，并创建一个名称为“zzk”的database，然后创建一个指定结构的表，名称为“account_password”  
MySQL的安装见：https://github.com/Fengxingzhe666/mysql_practice  
这里建议安装时选择默认路径，否则项目中的附加包含还需修改为自定义的安装路径。  
## 服务器端的MySQL简单配置过程  
```
PS C:\Users\username\Desktop>mysql -uroot -p
Enter password: ******
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 15
Server version: 5.6.29-log MySQL Community Server (GPL)

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> create database zzk;
Query OK, 1 row affected (0.00 sec)

mysql> use zzk;
Database changed

mysql> create table account_password(
    -> account VARCHAR(20) PRIMARY KEY,
    -> password VARCHAR(20)
    -> );
Query OK, 0 rows affected (0.02 sec)

```

## g++编译工具的使用  
Windows系统安装gcc/g++：https://blog.csdn.net/happycell188/article/details/145774606  
（gcc和g++都是C语言的编译工具，g++更加针对c++，因此本项目主要使用g++）  
在本地下载后解压，并在高级系统设置->环境变量->系统变量->PATH->添加安装的gcc的文件夹bin路径，然后在命令行输入：g++ --version，如果能看到版本反馈信息，就说明安装成功了。  
```
PS D:\App\Private-network-online-drive-system> g++ --version
g++.exe (MinGW-W64 x86_64-ucrt-mcf-seh, built by Brecht Sanders, r1) 15.1.0
Copyright (C) 2025 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```
如果是服务端程序，则在命令行进入server文件夹，然后输入以下编译指令进行编译：  
```
g++ server.cpp ../ProgressBar.h ../ProgressBar.cpp ../handleAll.h ../handleAll.cpp -o server.exe -I "C:/Program Files/MySQL/MySQL Server 5.6/include" -L "C:/Program Files/MySQL/MySQL Server 5.6/lib"  -lmysql -lws2_32
```
g++会在server目录中编译出server.exe文件，运行这个exe文件，服务端就开始运行了。

如果是客户端程序，则在命令行进入client文件夹，然后输入以下编译指令进行编译：  
```
g++ client.cpp ../ProgressBar.h ../ProgressBar.cpp ../handleAll.h ../handleAll.cpp -o client.exe -lws2_32
```
g++会在client目录中编译出client.exe文件，运行这个exe文件，客户端就开始运行了。 

此外如果有Visual Studio，是可以直打开.vcxproj文件进行编译的，项目里的文件已经打包好了。  
 
2025.5.21  
