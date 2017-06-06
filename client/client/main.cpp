#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

unsigned short portnum=3431;

int Rfd, Sfd;
ssize_t recbytes;

char ip[64];
char RecvPack[1024]={0}, SendPack[1024]={0};    //SendPack为发送包 RecvPack为接受包
struct sockaddr_in s_add;

// &&& 分隔 type 和 content
// TYPE: 客户端主动发给服务器:1/2/3/4/5 服务器主动发给客户端:0（即帮另一台客户端转发）

void list_menu() {
    printf("Choose one option from menu below\n");
    printf("(1) disconnect from the server\n");
    printf("(2) receive current time\n");
    printf("(3) receive server information\n");
    printf("(4) receive active conection list of server\n");
    printf("(5) send message to specific client\n");
}

//制作发送请求包
int makeSendPack(int obj){
    //1:正常处理    0:disconnect    -1:error input
    switch (obj) {
        case 1: {
            send(Sfd, "1&&&disconnect", 1024, 0);
            return 0;
        }
        case 2: {
            strcpy(SendPack, "2&&&get time");
            break;
        }
        case 3: {
            strcpy(SendPack, "3&&&get server's infomation");
            break;
        }
        case 4: {
            strcpy(SendPack, "4&&&get clients list");
            break;
        }
        case 5: {
            char buf_tmp[1024];
            strcpy(SendPack, "5&&&");
            
            printf("Input your object's ip.\n");
            scanf("%s", buf_tmp);
            getchar();
            strcat(SendPack, buf_tmp);
            strcat(SendPack, "&&&");
            
            printf("Input your sentences.\n");
            fgets(buf_tmp, 1024, stdin);
            strcat(SendPack, buf_tmp);
            break;
        }
        default: {
            printf("error input. please input again\n");
            return -1;
        }
    }
    return 1;
}

//处理接收响应包
void dealRecvPack(){
    switch (RecvPack[0]) {
        case '0': {
            printf("%s\n", RecvPack+4);
            send(Rfd, "0&&&get server's redirected package.\n", 1024, 0);
            break;
        }
        case '2': case '3': case '4': case '5':{
            //response package
            printf("%s\n", RecvPack+4);
            break;
        }
        default:{
            printf("error type package.\n");
            break;
        }
    }
}

//socket1:等待发送状态，用于发送包给服务器（即type=1、2、3、4、5的包）
void* SendSock(void* args){
    int obj, length;
    list_menu();
    
    while (1) {
        scanf("%d", &obj);
        getchar();
        
        if(makeSendPack(obj) == 1){
            send(Sfd, SendPack, 1024, 0);
            length = (int)recv(Sfd, RecvPack, 1024, 0);
            RecvPack[length] = '\0';
            if(length){
                dealRecvPack();
            }
        }else if(makeSendPack(obj) == 0){
            length = (int)recv(Sfd, RecvPack, 1024, 0);
            RecvPack[length] = '\0';
            if(length){
                printf("%s\n", RecvPack+4); //接收包RecvPack case 1
                return NULL;
            }
        }else if(makeSendPack(obj) == -1){
            continue;
        }
        sleep(2);
    }
    
    return NULL;
}

//socket2:等待接收状态，用于接收其他客户端发来的包（即type=0，服务器主动发给我的包）
void* RecSock(void* args){
    int length;
    
    while (1) {
        length = (int)recv(Rfd, RecvPack, 1024, 0);
        RecvPack[length] = '\0';
        if(length){
            dealRecvPack();
        }
        sleep(2);
    }

    return NULL;
}

int main()
{
    int obj;
    printf("Choose one option from menu below\n");
    printf("(1) connect to a server.\n");
    printf("(0) exit\n");

    scanf("%d", &obj);
    
    if(obj == 0){
        return 0;
    }
    
    printf("ip server: \n");
    scanf("%s", ip);
    
    
    //创建发送、接受两个socket
    Rfd = socket(AF_INET, SOCK_STREAM, 0);
    Sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == Rfd || -1 == Sfd)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    
    //服务器网络地址
    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=inet_addr(ip);
    s_add.sin_port=htons(portnum);
    printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);
    
    //尝试将发送和接收socket连接至服务器
    if((-1 == connect(Rfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))||-1 == connect(Sfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("connect fail !\r\n");
        return -1;
    }
    printf("connect ok !\r\n");
    
    //连接成功，线程处理数据
    pthread_t Sp, Rp;
    pthread_create(&Sp, NULL, SendSock, NULL);
    pthread_create(&Rp, NULL, RecSock, NULL);
    
    pthread_join(Sp, NULL);

    return 0;
}

