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
#include <math.h>

#define PORT 3431
#define SERVER_NAME "Li Mu and Chen Ran's Lab7 Server."

struct sockaddr_in s_addr;  //服务器网络地址
unsigned int sin_size;  //服务器网络地址长度
int sfp, count; //服务器socket标志符  客户端数量count

//客户端结构
struct client{
    int id;
    int rfd, sfd;   //接受端、发送端socket标志符
    struct sockaddr_in c_addr;  //客户端socket网络地址
    char rbuf[1024], sbuf[1024];    //接受端、发送端缓存区
    pthread_t s_process, r_process; //接受端、发送端线程
    int flag;
}c[10]; //所有客户端结构数组

//int转char
void inttochar(int x, char str[]){
    int i, count, result = 1;
    for(i = 0; (int)result > 0; i++){
        result = (x / pow(10, i));
    }
    count = i - 1;
    for(i = 0; i < count; i++){
        int plus = (int)pow(10, (count-1-i));
        str[i] = (x / plus) + '0';
        x = x%plus;
    }
    str[count] = '\0';
}

//客户端线程id，包字符串pck
//TYPE: request:1/2/3/4/5 response:0
void dealPack(int id, char* pck){
    int i;
    char resPck[1024] = {0};
    
    switch (pck[0]) {
        case '0':{
            //response package
            printf("client %d %s", id, pck+4);
            return ;
        }
        case '1':{
            c[id].flag = 0;
            strcpy(resPck, "1&&&disconnect success.");
            break;
        }
        case '2':{
            time_t timep;
            time(&timep);
            strcpy(resPck, "2&&&");
            strcat(resPck, ctime(&timep));
            break;
        }
        case '3':{
            strcpy(resPck, "3&&&");
            strcat(resPck, SERVER_NAME);
            strcat(resPck, "\n");
            break;
        }
        case '4':{
            char output[1024];
            int length = 0;
            
            for(i = 0; i < 10; i++)
            {
                if(c[i].flag)
                {
                    length += sprintf(output+length, "client %d: %s\n", i, inet_ntoa(c[i].c_addr.sin_addr));
                }
            }
            strcpy(resPck, "4&&&");
            strcat(resPck, output);
            break;
        }
        case '5':{
            char ip[100], message[1024];
            char *a = strstr(pck+4,"&&&");
            strncpy(ip, pck+4, a-pck-4);
            strcpy(message,a+3);
            
            for(i = 0; i < 10; i++)
            {
                if(strcmp(inet_ntoa(c[i].c_addr.sin_addr),ip) == 0 && c[i].flag == 1)
                {
                    char output[1024];
                    int length = 0;
                    
                    printf("client %d want to send message to client %d.\n", id, i);
                    sprintf(output, "0&&&From client %d(%s): %s", id, ip, message);
                    length = (int)send(c[i].sfd, output, 1024, 0);
                    length = (int)recv(c[i].sfd, c[i].sbuf, 1024, 0); //得到响应
                    c[i].sbuf[length] = '\0';
                    dealPack(i, c[i].sbuf);
                    strcpy(resPck, "5&&&send to this client success\n");
                    break;
                }
            }
            break;
        }
        default:{
            printf("%s\n", pck);
            printf("error type package.\n");
            break;
        }
    }
    send(c[id].rfd,resPck,1024,0);
    if(pck[0] == '1' || pck[0] == '2' || pck[0] == '3' || pck[0] == '4'){
        printf("client %d want to %s.\n", id, pck+4);
    }
}

//处理线程函数
void* RecSock(void* args){
    //客户端id
    int id = *(int *)args;
    char *pck;

    while (c[id].flag) {
        if(recv(c[id].rfd, c[id].rbuf, 1024, 0)>0){   //接受数据
            pck = c[id].rbuf;
            if((int)strlen(pck) > 0) dealPack(id, pck);
            else printf("空数据流:%d\n", (int)strlen(pck));
        }
        sleep(2);
    }
    
    return NULL;
}

int main(){
    //创建一个socket
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    
    //创建服务器ip、port等网络数据
    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family=AF_INET;
    s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    s_addr.sin_port=htons(PORT);
    
    //将服务器网络数据与socket绑定
    if(-1 == bind(sfp,(struct sockaddr *)(&s_addr), sizeof(struct sockaddr)))
    {
        printf("bind fail !\r\n");
        return -1;
    }
    
    //将socket加入监听队列
    if(-1 == listen(sfp,5))
    {
        printf("listen fail !\r\n");
        return -1;
    }
    
    sin_size = sizeof(struct sockaddr_in);
    printf("Server start, Socket start.\n");
    
    //循环等待客户端链接
    while (1) {
        int id;
        //accept阻塞，等待客户端连接
        c[count].sfd = accept(sfp, (struct sockaddr *)(&c[count].c_addr), &sin_size);
        c[count].rfd = accept(sfp, (struct sockaddr *)(&c[count].c_addr), &sin_size);
        if(-1 == c[count].sfd || -1 == c[count].rfd)
        {
            printf("accept fail !\r\n");
            return -1;
        }
        id = count;
        c[id].flag= 1;
        printf("client %d connect. ip: %x\n", id, c[count].c_addr.sin_addr.s_addr);
        
        //创建客户端发送、接受处理线程
        pthread_create(&c[count].r_process, NULL, RecSock, &id);
        count++;
    }
    
}
