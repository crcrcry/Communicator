# Socket Communicator
- 利用 socket 实现 客户端-服务器-客户端 的通信

## 具体功能
### 客户端
- a) 连接：请求连接到指定地址和端口的服务端
- b) 断开连接：断开与服务端的连接
- c) 获取时间: 请求服务端给出当前时间
- d) 获取名字：请求服务端给出其机器的名称
- e) 活动连接列表：请求服务端给出当前连接的所有客户端信息（编号、IP地址、端口等）
- f) 发消息：请求服务端把消息转发给对应编号的客户端，该客户端收到后显示在屏幕上
- g) 退出：断开连接并退出客户端程序

### 服务器
- a) 向客户端传送服务端所在机器的当前时间
- b) 向客户端传送服务端所在机器的名称
- c) 向客户端传送当前连接的所有客户端信息
- d) 将某客户端发送过来的内容转发给指定编号的其他客户端
- e) 采用异步多线程编程模式，正确处理多个客户端同时连接，同时发送消息的情况

## 主要数据结构
### 服务器端的客户端列表
```c
//客户端结构
struct client
{
    int id;
    int rfd, sfd;   //接受端、发送端socket标志符
    struct sockaddr_in c_addr;  //客户端socket网络地址
    char rbuf[1024], sbuf[1024];    //接受端、发送端缓存区
    pthread_t s_process, r_process; //接受端、发送端线程
}c[10]; //所有客户端结构数组
```

### 网络地址
```c
struct sockaddr_in
{
    short sin_family;   //协议类型，用AF_INET
    unsigned short sin_port;    //端口号
    struct in_addr sin_addr;    //ip地址
    unsigned char sin_zero[8];  //作用不大不用管
}
```

### 数据包
```c
char pck[3][];
//  type: pck[0]: 功能类型，有以下几种选项
//      1. disconnect：断开连接
//      2. time：获取时间
//      3. server_info：获取服务器信息
//      4. client_list：获取连接客户端列表
//      5. msg：发消息给其他客户端
//  content: pck[1]：包的内容文本，功能功能1-4可填空字符串，功能5必填
//  url: pck[2]：包的转发地址，功能1-4可填空字符串，功能5必填
//  send和recv的时候强制类型转换发送
```
