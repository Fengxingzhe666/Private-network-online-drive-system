# 测试server-client的tcp-ipv4/6连接  
修改原来v1.0中tcp-ipv4连接方式为tcp-ipv4/6，兼容两种版本的ip协议，已经通过本机回环测试。  
本branch用于检测server-client的tcp-ipv4/6连接是否能走通，使用时把client端的连接地址改为server端的公网ipv4/6地址，一定要先在server端编译运行server.cpp，再在client端编译运行client.cpp
