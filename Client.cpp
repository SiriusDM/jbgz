#include <winsock2.h>
#include <iostream>
#include <stdio.h>
#include <cstring>

using namespace std;

#pragma comment (lib, "ws2_32.lib");


const int BUFFER_SIZE=1024;
DWORD WINAPI recvMsgThread(LPVOID lpParameter);

int main(int argc, char *argv[]) {
    WORD wVersionRequested = MAKEWORD(2,2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested,&wsaData) != 0)
        return 0;
    SOCKET sClient = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sClient == INVALID_SOCKET) {
        printf("Invalid Socket !");
        return 0;
    }
    sockaddr_in ServAddr;
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_port = htons(2020);
    ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sClient,(sockaddr *)&ServAddr,sizeof(ServAddr)) == SOCKET_ERROR) { //向服务器请求连接
        printf("Connect Error !"); 
        return 0;
    }

	CloseHandle(CreateThread(NULL,0,recvMsgThread,(LPVOID)&sClient,0,0)); //创建消息处理线程

	while (1) {
		char buf[BUFFER_SIZE] = { 0 };
		cin.getline(buf,sizeof(buf));
		if (strcmp(buf, "exit") == 0) {
			break;
		}
		send(sClient,buf,sizeof(buf),0);
	}
//    printf("Connect Success!\n");
/*    char  cData[255];
	cin>>cData;
//	printf("%s\n",cData);
    while (send(sClient,cData,strlen(cData),0)>0){
         char  rData[255];
         int ret = recv(sClient,rData,255,0);
         if (ret > 0) {
             rData[ret] = '\0';
             printf(rData);
             printf("\n");
        }
        cin>>cData;
    }*/
	closesocket(sClient);
    WSACleanup();
    return 0;
}
DWORD WINAPI recvMsgThread(LPVOID lpParameter) {
	SOCKET clSock = *(SOCKET*)lpParameter;

	while (1) {
		char buf[BUFFER_SIZE] = { 0 };
		int nrecv = recv(clSock, buf, sizeof(buf), 0);
		if (nrecv > 0) {
			cout << buf << endl;
		}
		else if (nrecv < 0){
			cout << "Fail to Connect Server !"<<endl;
			break;
		}
	}
	return 0;
}
