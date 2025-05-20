# 模拟私有云盘系统  
**TCP** **SOCKET编程** **应用层文件传输协议**  
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
```
-d、-i、-u命令内容待完善……  
2025.5.20  
