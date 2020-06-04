#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <cstring>


using namespace std;

#pragma comment(lib,"ws2_32.lib");

const int WAIT_TIME = 10;
const int MAX_CLIENT_NUM = 10;
const int BUFFER_SIZE = 1024;
int RCV_WAIT_TIMEOUT = 10;
int SND_WAIT_TIMEOUT = 10;


SOCKADDR_IN clAddr[MAX_CLIENT_NUM];
SOCKET clSock[MAX_CLIENT_NUM];
WSAEVENT clEvent[MAX_CLIENT_NUM];  //0->Server
int now_link_num=0;


DWORD  WINAPI servEventThread(LPVOID lpParameter); //事件选择线程

int main(int argc,char* argv[]) {
    //初始化WSA
    WORD wVersionRequested = MAKEWORD(2,2); //2.2版本Socket
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested,&wsaData)!=0) 
        return 0; //初始化失败
    //创建Socket
    SOCKET ServSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);//IPv4,TCP
    if (ServSock == INVALID_SOCKET){ //WINSOCK2中INVALID_SOCKET的值是(~0)
        printf("Socket Error !");
        return 0;
    }
    //Bind
    sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET; //IPv4
    SockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");//将字符串形式的IP地址转换成网络字节序
    SockAddr.sin_port = htons(2020);//使用端口2020
    if (bind(ServSock,(SOCKADDR*)&SockAddr,sizeof(SockAddr)) == SOCKET_ERROR) {
        printf("Bind Error !");
        return 0;
    }
    //Listen
    if (listen(ServSock,10) == SOCKET_ERROR) {
        printf("Listen Error !");
        return 0;
    }
    //创建accept的线程
    WSAEVENT servEvent =WSACreateEvent();
    WSAEventSelect(ServSock,servEvent,FD_ALL_EVENTS);//选择事件对象并监听所有事件
    clSock[0] = ServSock;
    clEvent[0] = servEvent;
    CloseHandle(CreateThread(NULL,0,servEventThread,(LPVOID)&ServSock,0,0));
	while (1) {
		char buf[BUFFER_SIZE] = { 0 };
		cin.getline(buf, sizeof(buf));
		for (int j = 1; j <= now_link_num; j++)
			send(clSock[j],buf,sizeof(buf),0);
	}
/*    //收发数据测试
    SOCKET sClient;
    sockaddr_in cAddr;
    int nAddr = sizeof(cAddr);
       printf("waiting connect...");
        sClient = accept(ServSock,(SOCKADDR*)&cAddr,&nAddr); //accrpt当前请求
        if (sClient == INVALID_SOCKET) { 
            printf("Accept Error !");
        }
                printf("connet from: %s \r\n",inet_ntoa(cAddr.sin_addr));
    while (1) {
        //接收数据
        char cData[255]={};
        int ret = recv(sClient, cData, 255 ,0);
//        printf(cData);
        char sData[]="Server:";
        if (ret > 0) {
            strcat(sData,cData);
            cData[ret] = '\0';
//            printf("Server:");
            printf(sData);
            printf("\n");
        }
        //发送数据
        send(sClient,sData,strlen(sData),0);
    } */
    //   closesocket(sClient);
    closesocket(ServSock);
    WSACleanup();
    return 0;
}

DWORD  WINAPI servEventThread(LPVOID lpParameter) {//事件选择线程
    SOCKET servSock = *(SOCKET*)lpParameter; //将其转换为SOCKET*类型再处理
    while (1) {  // 循环监听检查
        for (int i=0;i<now_link_num+1;i++) {
            int tp = WSAWaitForMultipleEvents(1,&clEvent[i],false, WAIT_TIME,0);
            tp -= WSA_WAIT_EVENT_0;
//			cout << tp << endl;
			if (tp == WSA_WAIT_FAILED || tp == WSA_WAIT_TIMEOUT) { //出错or超时
		//		cout << tp << endl;
				continue;
			}
            if (tp == 0) {   //Server发生事件
			//	cout << now_link_num << endl;
                WSANETWORKEVENTS netEvent;
                WSAEnumNetworkEvents(clSock[i],clEvent[i],&netEvent);
			//	cout << k << endl;
                if (netEvent.lNetworkEvents & FD_ACCEPT){  //ACCEPT
				//	cout << '1' << endl;
                    if (netEvent.iErrorCode[FD_ACCEPT_BIT]) {
                        printf("User %d Wrong !\n",i);
                        continue;
                    }
                  int AddrLen = sizeof(SOCKADDR);
                  SOCKET nSock = accept(servSock,(SOCKADDR*)&clAddr[now_link_num+1],&AddrLen);
                  if (nSock == INVALID_SOCKET) continue;
				  //设置超时机制
				  setsockopt(nSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&SND_WAIT_TIMEOUT, sizeof(SND_WAIT_TIMEOUT));
				  setsockopt(nSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&RCV_WAIT_TIMEOUT, sizeof(RCV_WAIT_TIMEOUT));
                  clSock[now_link_num + 1] = nSock;
                  WSAEVENT nEvent = WSACreateEvent(); //创建新的事件对象
                  WSAEventSelect(clSock[now_link_num+1],nEvent,FD_CLOSE|FD_READ|FD_WRITE);// 监听事件种类
				  clEvent[now_link_num + 1] = nEvent;
                  now_link_num++;//连接数+1
                  printf("User %d Enter the Chatting Room !\n",now_link_num	);
				  char buf[BUFFER_SIZE] = "[Server]:Welcome User(IP:";
				  strcat(buf,inet_ntoa(clAddr[now_link_num].sin_addr));
				  strcat(buf,") Enter the Chatting Room !");
			      for (int j = 1; j <= now_link_num; j++)
					send(clSock[j],buf,sizeof(buf),0);
                } 
				else if (netEvent.lNetworkEvents & FD_CLOSE) {//close
				//	cout << '1' << endl;
					printf("User %d Exit The Chatting Room !\n", i);
					closesocket(clSock[i]);
					WSACloseEvent(clEvent[i]); //释放资源

					//以数组序号储存用户信息，有上限，需要调整
					for (int j = i; j < now_link_num; j++) {
						clSock[j] = clSock[j + 1];
						clEvent[j] = clEvent[j + 1];
						clAddr[j] = clAddr[j + 1];
					}
					char buf[BUFFER_SIZE] = "[Server]:User(IP:";
					strcat(buf, inet_ntoa(clAddr[now_link_num].sin_addr));
					strcat(buf, ") Exit the Chatting Room !");
					for (int j = 1; j < now_link_num; j++)
						send(clSock[j], buf, sizeof(buf), 0);
					
					now_link_num--;
				}
				else if (netEvent.lNetworkEvents & FD_READ) {//read 接收消息 并向其他client转发
					char buf[BUFFER_SIZE] = { 0 };
					char buf2[BUFFER_SIZE] = { 0 };
					int nrecv = recv(clSock[i],buf,sizeof(buf),0);
					if (nrecv > 0) {
						sprintf(buf2,"[User%d]:%s",i,buf);
						cout << buf2 << endl;
						for (int j = 1; j <= now_link_num; j++) {
							send(clSock[j],buf2,sizeof(buf2),0);
						}
					}
				}
			//	cout << '2' << endl;
            }
        }
    

    }
    return 0;
}
