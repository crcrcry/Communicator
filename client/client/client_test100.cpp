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

int Rfd, Sfd;
ssize_t recbytes;

char ip[64];
char Rbuf[1024]={0}, Sbuf[1024]={0};
struct sockaddr_in s_add;
unsigned short portnum=3296;

// &&& 分隔
char pck[4096];

void list_menu() {
    printf("Choose one option from menu below\n");
    printf("(1) disconnect from the server\n");
    printf("(2) receive current time\n");
    printf("(3) receive server information\n");
    printf("(4) receive active conection list of server\n");
    printf("(5) send message to specific client\n");
}

void* SendSock(void* args){
    int obj, sum = 0;
    //list_menu();
    
    scanf("%d", &obj);
    getchar();
    for(int i = 0; i < obj; i++){
        strcpy(Sbuf, "2");
        send(Sfd, Sbuf, 4096, 0);
        if(recv(Sfd, Sbuf, 4096, 0)){
            printf("Send Success.\n");
            puts(Sbuf);
            sum++;
        }else{
            printf("Send Fail.\n");
        }
    }
    printf("sum = %d\n", sum);
    /*
    while (1) {
        
        sleep(2);
        scanf("%d", &obj);
        getchar();
        switch (obj) {
            case 1: {
                send(Sfd, "1", 10, 0);
                return NULL;
            }
            case 2: {
                strcpy(Sbuf, "2");
                break;
            }
            case 3: {
                strcpy(Sbuf, "3");
                break;
            }
            case 4: {
                strcpy(Sbuf, "4");
                break;
            }
            case 5: {
                char buf_tmp[4096];
                strcpy(Sbuf, "5&&&");
                
                printf("Input your object's ip.\n");
                scanf("%s", buf_tmp);
                getchar();
                strcat(Sbuf, buf_tmp);
                strcat(Sbuf, "&&&");
                
                printf("Input your sentences.\n");
                fgets(buf_tmp, 4096, stdin);
                strcat(Sbuf, buf_tmp);
                break;
            }
            default: {
                printf("error input. please input again\n");
                continue;
            }
        }
        
        send(Sfd, Sbuf, 4096, 0);
        if(recv(Sfd, Sbuf, 4096, 0)){
            printf("Send Success.\n");
            puts(Sbuf);
        }
    }
    */
    return NULL;
}

void* RecSock(void* args){
    
    while (1) {
        sleep(2);
        strcpy(Rbuf, "");
        if(recv(Rfd, Rbuf, 1024, 0)){
            printf("%s", Rbuf);
            send(Rfd, "200", 32, 0);
        }
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

